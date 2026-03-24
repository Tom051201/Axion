#include "axpch.h"
#include "Project.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/AxionSettings.h"

namespace Axion {

	// LAYOUT
	//
	// Project: NAME
	// Version: VERSION
	// EngineVersion: VERSION
	// Author: AUTHOR
	// Company: COMPANY
	// Description: DESC
	// ProjectPath: PATH
	// AssetsPath: PATH
	// ScenesPath: PATH
	// DefaultScene: PATH
	//
	// LAYOUT

	Project::Project(const std::string& name)
		: m_name(name), m_projectPath("Unknown"), m_assetsPath("Unknown") {
	
		m_assetRegistry = std::make_shared<AssetRegistry>();
	}

	Ref<Project> Project::load(const std::string& path) {
		// ----- Load file data -----
		std::ifstream stream(path);
		if (!stream.is_open()) {
			AX_CORE_LOG_WARN("Failed to open project file: {}", path);
			return nullptr;
		}
		YAML::Node data = YAML::Load(stream);
		if (!data["Project"]) { AX_CORE_LOG_WARN("Loading project file failed"); return nullptr; }

		Ref<Project> project = std::make_shared<Project>("");

		// ----- Set name and versions -----
		project->setName(data["Project"].as<std::string>());
		project->setVersion(data["Version"].as<std::string>());
		project->setEngineVersion(data["EngineVersion"].as<std::string>());

		// ----- Author, company and description -----
		if (data["Author"]) project->setAuthor(data["Author"].as<std::string>());
		if (data["Company"]) project->setCompany(data["Company"].as<std::string>());
		if (data["Description"]) project->setDescription(data["Description"].as<std::string>());

		// ----- Project-, Assets- and ScenesPath -----
		project->setProjectPath(std::filesystem::path(path).parent_path().string());
		if (data["AssetsPath"]) project->setAssetsPath(data["AssetsPath"].as<std::string>());

		// ----- Default scene -----
		if (data["DefaultScene"]) project->setDefaultScene(data["DefaultScene"].as<std::string>());

		std::string registryPath = (std::filesystem::path(path).parent_path() / "AssetRegistry.yaml").string();
		project->getAssetRegistry()->deserialize(registryPath);

		return project;
	}

	Ref<Project> Project::loadBinary(const std::string& path) {
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (!in.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open binary project config file: {}", path);
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
		std::string name(nameLength, '\0');
		in.read(reinterpret_cast<char*>(&name[0]), nameLength);

		// -- Read Default Scene UUID --
		UUID defaultSceneUUID;
		in.read(reinterpret_cast<char*>(&defaultSceneUUID), sizeof(UUID));

		in.close();
		AX_CORE_LOG_INFO("Successfully Loaded GameConfig Binary");

		// -- Construct the Runtime Project --
		Ref<Project> project = std::make_shared<Project>(name);

		project->setProjectPath(".");
		project->setAssetsPath(".");
		project->setDefaultSceneUUID(defaultSceneUUID);
		project->getAssetRegistry()->deserializeBinary("AssetRegistry.bin");

		return project;
	}

	void Project::save(const std::string& path) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Project" << YAML::Value << m_name;
		out << YAML::Key << "Version" << YAML::Value << m_version;
		out << YAML::Key << "EngineVersion" << YAML::Value << m_engineVersion;

		if (!m_author.empty()) out << YAML::Key << "Author" << YAML::Value << m_author;
		if (!m_company.empty()) out << YAML::Key << "Company" << YAML::Value << m_company;
		if (!m_description.empty()) out << YAML::Key << "Description" << YAML::Value << m_description;

		out << YAML::Key << "ProjectPath" << YAML::Value << m_projectPath;
		out << YAML::Key << "AssetsPath" << YAML::Value << m_assetsPath;

		if (!m_defaultScene.empty()) out << YAML::Key << "DefaultScene" << YAML::Value << m_defaultScene;

		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();

		std::string registryPath = (std::filesystem::path(path).parent_path() / "AssetRegistry.yaml").string();
		m_assetRegistry->serialize(registryPath);
	}

	Ref<Project> Project::createNew(const ProjectSpecification& spec) {
		namespace fs = std::filesystem;

		std::string projectName = spec.name;
		std::replace(projectName.begin(), projectName.end(), ' ', '_');
		Ref<Project> result = std::make_shared<Project>(spec.name);

		try {
			fs::path projectDir = fs::path(spec.location) / projectName;

			// create project dir
			if (!fs::exists(projectDir)) {
				fs::create_directories(projectDir);
			}

			// create assets dir
			fs::path assetsDir = projectDir / "Assets";
			fs::create_directories(assetsDir);

			// setup project
			result->setName(spec.name);
			result->setVersion("0.1.0"); // TODO: make project version configurable
			result->setEngineVersion(AX_ENGINE_VERSION);
			result->setProjectPath(projectDir.string());
			result->setAssetsPath(assetsDir.string());

			if (!spec.author.empty()) result->setAuthor(spec.author);
			if (!spec.company.empty()) result->setCompany(spec.company);
			if (!spec.description.empty()) result->setDescription(spec.description);

			// write the project file (.axproj)
			result->save((projectDir / (projectName + ".axproj")).string());

			std::string registryPath = (projectDir / "AssetRegistry.yaml").string();
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
