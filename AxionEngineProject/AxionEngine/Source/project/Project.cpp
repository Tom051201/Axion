#include "axpch.h"
#include "Project.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/EngineConfig.h"

namespace Axion {

	Project::Project(const std::string& name)
		: m_name(name) {
	
		m_assetRegistry = std::make_shared<AssetRegistry>();
	}

	Ref<Project> Project::load(const std::filesystem::path& path) {
		// ----- Load file data -----
		std::ifstream stream(path);
		if (!stream.is_open()) {
			AX_CORE_LOG_WARN("Failed to open project file: {}", path.string());
			return nullptr;
		}
		YAML::Node data = YAML::Load(stream);
		if (!data["Project"]) {
			AX_CORE_LOG_WARN("Loading project file failed");
			return nullptr;
		}

		Ref<Project> project = std::make_shared<Project>("");

		// ----- Set name and versions -----
		project->setName(data["Project"].as<std::string>());
		project->setVersion(Version::fromString(data["Version"].as<std::string>()));
		project->setEngineVersion(Version::fromString(data["EngineVersion"].as<std::string>()));

		// ----- Author, company and description -----
		if (data["Author"]) project->setAuthor(data["Author"].as<std::string>());
		if (data["Company"]) project->setCompany(data["Company"].as<std::string>());
		if (data["Description"]) project->setDescription(data["Description"].as<std::string>());

		// ----- Project-, Assetspath -----
		project->setProjectPath(path.parent_path());
		if (data["AssetsPath"]) project->setAssetsPath(data["AssetsPath"].as<std::string>());

		// ----- Default scene -----
		if (data["DefaultScene"]) project->setDefaultScene(data["DefaultScene"].as<std::string>());
		if (data["AppIcon"]) project->setAppIconPath(data["AppIcon"].as<std::string>());

		std::filesystem::path registryPath = path.parent_path() / "AssetRegistry.yaml";
		project->getAssetRegistry()->deserialize(registryPath);

		return project;
	}

	Ref<Project> Project::loadBinary(const std::filesystem::path& path) {
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (!in.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open binary project config file: {}", path.string());
			return nullptr;
		}

		char magic[4];
		in.read(magic, 4);
		if (memcmp(magic, "AXCF", 4) != 0) {
			AX_CORE_LOG_ERROR("Invalid GameConfig Binary Signature!");
			return nullptr;
		}

		uint32_t version;
		in.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));

		// -- Read Game Name --
		uint32_t nameLength;
		in.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));

		if (nameLength > Config::MaxBinaryStringLength) {
			AX_CORE_LOG_FATAL("GameConfig Binary Corrupted: Project name length ( {} ) exceeds maximum limit of {} bytes!", nameLength, Config::MaxBinaryStringLength);
			return nullptr;
		}

		std::string name(nameLength, '\0');
		in.read(reinterpret_cast<char*>(&name[0]), nameLength);

		// -- Read Default Scene UUID --
		UUID defaultSceneUUID;
		in.read(reinterpret_cast<char*>(&defaultSceneUUID), sizeof(UUID));

		// -- Read App Icon Path --
		uint32_t iconPathLength;
		in.read(reinterpret_cast<char*>(&iconPathLength), sizeof(uint32_t));

		if (iconPathLength > Config::MaxBinaryStringLength) {
			AX_CORE_LOG_FATAL("GameConfig Binary Corrupted: App icon path length ({}) exceeds {} bytes!", iconPathLength, Config::MaxBinaryStringLength);
			return nullptr;
		}

		std::string iconPath(iconPathLength, '\0');
		if (iconPathLength > 0) {
			in.read(&iconPath[0], iconPathLength);
		}

		in.close();
		AX_CORE_LOG_INFO("Successfully Loaded GameConfig Binary");

		// -- Construct the Runtime Project --
		Ref<Project> project = std::make_shared<Project>(name);

		project->setProjectPath(".");
		project->setAssetsPath(".");
		project->setDefaultSceneUUID(defaultSceneUUID);
		project->getAssetRegistry()->deserializeBinary("AssetRegistry.bin"); // TODO: maybe move this inside the assets folder or a config folder
		project->setAppIconPath(iconPath);

		return project;
	}

	void Project::save(const std::filesystem::path& path) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Project" << YAML::Value << m_name;
		out << YAML::Key << "Version" << YAML::Value << m_version.toString();
		out << YAML::Key << "EngineVersion" << YAML::Value << m_engineVersion.toString();

		if (!m_author.empty()) out << YAML::Key << "Author" << YAML::Value << m_author;
		if (!m_company.empty()) out << YAML::Key << "Company" << YAML::Value << m_company;
		if (!m_description.empty()) out << YAML::Key << "Description" << YAML::Value << m_description;

		out << YAML::Key << "ProjectPath" << YAML::Value << m_projectPath.generic_string();
		out << YAML::Key << "AssetsPath" << YAML::Value << m_assetsPath.generic_string();

		if (!m_defaultScene.empty()) out << YAML::Key << "DefaultScene" << YAML::Value << m_defaultScene.generic_string();
		if (!m_appIconPath.empty()) out << YAML::Key << "AppIcon" << YAML::Value << m_appIconPath.generic_string();

		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();

		std::filesystem::path registryPath = path.parent_path() / "AssetRegistry.yaml";
		m_assetRegistry->serialize(registryPath);
	}

	Ref<Project> Project::createNew(const ProjectSpecification& spec) {
		namespace fs = std::filesystem;

		std::string projectName = spec.name;
		std::replace(projectName.begin(), projectName.end(), ' ', '_');
		Ref<Project> result = std::make_shared<Project>(spec.name);

		try {
			fs::path projectDir = spec.location / projectName;

			// -- Create Project Directory --
			if (!fs::exists(projectDir)) {
				fs::create_directories(projectDir);
			}


			// -- Create Assets Directory --
			fs::path assetsDir = projectDir / "Assets";
			fs::create_directories(assetsDir);


			// -- Create Export Directory --
			fs::path exportDir = projectDir / "Export";
			fs::create_directories(exportDir);


			// -- Setup Project --
			result->setName(spec.name);
			result->setVersion(spec.version);
			result->setEngineVersion(Config::EngineVersion);
			result->setProjectPath(projectDir);
			result->setAssetsPath(assetsDir);

			if (!spec.author.empty()) result->setAuthor(spec.author);
			if (!spec.company.empty()) result->setCompany(spec.company);
			if (!spec.description.empty()) result->setDescription(spec.description);

			// -- Write the .axproj File --
			result->save(projectDir / (projectName + ".axproj"));

			std::filesystem::path registryPath = (projectDir / "AssetRegistry.yaml");
			result->getAssetRegistry()->serialize(registryPath);

			return result;
		}
		catch (std::exception& e) {
			AX_CORE_LOG_ERROR("Failed to create project: {}", e.what());
			(void)e;
			return nullptr;
		}
	}

}
