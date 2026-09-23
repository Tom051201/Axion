#include "axpch.h"
#include "Project.h"

#include <yaml-cpp/yaml.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/PathResolver.h"
#include "AxionEngine/Source/physics/PhysicsLayerManager.h"

namespace Axion {

	Project::Project(const std::string& name)
		: m_name(name) {
	
		m_assetRegistry = MakeShared<AssetRegistry>();
	}

	Shared<Project> Project::load(const std::filesystem::path& path) {
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

		Shared<Project> project = std::make_shared<Project>("");

		// ----- Set name and versions -----
		project->setName(data["Project"].as<std::string>());
		project->setVersion(Version::fromString(data["Version"].as<std::string>()));
		project->setEngineVersion(Version::fromString(data["EngineVersion"].as<std::string>()));

		// ----- Author, company and description -----
		if (data["Author"]) project->setAuthor(data["Author"].as<std::string>());
		if (data["Company"]) project->setCompany(data["Company"].as<std::string>());
		if (data["Description"]) project->setDescription(data["Description"].as<std::string>());

		// ----- Path Resolution Chain -----
		std::filesystem::path absProjectDir = std::filesystem::absolute(path.parent_path());
		project->setProjectPath(absProjectDir);
		PathResolver::setAlias("{projdir}", absProjectDir);

		if (data["AssetsPath"]) {
			std::string rawAssets = data["AssetsPath"].as<std::string>();
			std::filesystem::path resolvedAssets = PathResolver::resolve(rawAssets);
			project->setAssetsPath(resolvedAssets);
			AX_CORE_LOG_TRACE("VFS: Resolved AssetsPath [{0}] -> [{1}]", rawAssets, resolvedAssets.string());
		}
		else {
			project->setAssetsPath(absProjectDir / "Assets");
		}

		PathResolver::setAlias("{assetsdir}", project->getAssetsPath());

		if (data["DefaultScene"]) {
			std::string rawScene = data["DefaultScene"].as<std::string>();
			std::filesystem::path resolvedScene = PathResolver::resolve(rawScene);
			project->setDefaultScene(resolvedScene);
			AX_CORE_LOG_TRACE("VFS: Resolved DefaultScene [{0}] -> [{1}]", rawScene, resolvedScene.string());
		}

		if (data["AppIcon"]) {
			std::string rawIcon = data["AppIcon"].as<std::string>();
			std::filesystem::path resolvedIcon = PathResolver::resolve(rawIcon);
			project->setAppIconPath(resolvedIcon);
			AX_CORE_LOG_TRACE("VFS: Resolved AppIcon [{0}] -> [{1}]", rawIcon, resolvedIcon.string());
		}

		if (data["PhysicsLayers"]) {
			PhysicsLayerManager::deserialize(data["PhysicsLayers"]);
		}
		else {
			PhysicsLayerManager::initialize();
		}

		std::filesystem::path registryPath = absProjectDir / "AssetRegistry.yaml";
		project->getAssetRegistry()->deserialize(registryPath);

		return project;
	}

	Shared<Project> Project::loadBinary(const std::filesystem::path& path) {
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

		PhysicsLayerManager::deserializeBinary(in);

		in.close();
		AX_CORE_LOG_INFO("Successfully Loaded GameConfig Binary");

		// -- Construct the Runtime Project --
		Shared<Project> project = MakeShared<Project>(name);

		project->setProjectPath(".");
		project->setAssetsPath(".");

		PathResolver::setAlias("{projdir}", ".");
		PathResolver::setAlias("{assetsdir}", ".");

		project->setDefaultSceneUUID(defaultSceneUUID);
		project->getAssetRegistry()->deserializeBinary("AssetRegistry.bin"); // TODO: maybe move this inside the assets folder or a config folder
		project->setAppIconPath(PathResolver::resolve(iconPath));

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

		std::string relAssets = std::filesystem::relative(m_assetsPath, m_projectPath).generic_string();
		out << YAML::Key << "AssetsPath" << YAML::Value << ("{projdir}/" + relAssets);

		if (!m_defaultScene.empty()) out << YAML::Key << "DefaultScene" << YAML::Value << PathResolver::virtualize(m_defaultScene);
		if (!m_appIconPath.empty()) out << YAML::Key << "AppIcon" << YAML::Value << PathResolver::virtualize(m_appIconPath);

		PhysicsLayerManager::serialize(out);

		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();

		std::filesystem::path registryPath = path.parent_path() / "AssetRegistry.yaml";
		m_assetRegistry->serialize(registryPath);
	}

	Shared<Project> Project::createNew(const ProjectSpecification& spec) {
		namespace fs = std::filesystem;

		std::string projectName = spec.name;
		std::replace(projectName.begin(), projectName.end(), ' ', '_');
		Shared<Project> result = std::make_shared<Project>(spec.name);

		try {
			fs::path projectDir = std::filesystem::absolute(spec.location / projectName);
			if (!fs::exists(projectDir)) fs::create_directories(projectDir);

			fs::path assetsDir = projectDir / "Assets";
			fs::create_directories(assetsDir);

			fs::path exportDir = projectDir / "Export";
			fs::create_directories(exportDir);

			Project::generateScriptProject(projectDir);

			// -- Setup Project --
			result->setName(spec.name);
			result->setVersion(spec.version);
			result->setEngineVersion(Config::EngineVersion);
			result->setProjectPath(projectDir);
			result->setAssetsPath(assetsDir);

			PathResolver::setAlias("{projdir}", projectDir);
			PathResolver::setAlias("{assetsdir}", assetsDir);

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

	void Project::generateScriptProject(const std::filesystem::path& projectDir) {
		namespace fs = std::filesystem;

		// -- Create Scripts Directory --
		fs::path scriptsDir = projectDir / "Scripts";
		if (!fs::exists(scriptsDir)) {
			fs::create_directories(scriptsDir);
		}

		// -- Create C# Project --
		fs::path csprojPath = scriptsDir / "GameAssembly.csproj";
		if (!fs::exists(csprojPath)) {
			fs::path engineDir = PlatformUtils::getExecutableDirectory();
			std::string coreDllPath = (engineDir / "AxionScriptCore.dll").generic_string();

			std::ofstream csprojFile(csprojPath);
			csprojFile << "<Project Sdk=\"Microsoft.NET.Sdk\">\n";
			csprojFile << "  <PropertyGroup>\n";
			csprojFile << "    <TargetFramework>net10.0</TargetFramework>\n";
			csprojFile << "    <ImplicitUsings>enable</ImplicitUsings>\n";
			csprojFile << "    <Nullable>enable</Nullable>\n";
			csprojFile << "  </PropertyGroup>\n";
			csprojFile << "  <ItemGroup>\n";
			csprojFile << "    <Reference Include=\"AxionScriptCore\">\n";
			csprojFile << "      <HintPath>" << coreDllPath << "</HintPath>\n";
			csprojFile << "    </Reference>\n";
			csprojFile << "  </ItemGroup>\n";
			csprojFile << "</Project>\n";
			csprojFile.close();
		}

		// -- Create Sample Script --
		fs::path sampleScriptPath = scriptsDir / "Player.cs";
		if (!fs::exists(sampleScriptPath)) {
			std::ofstream scriptFile(sampleScriptPath);
			scriptFile << "using System;\nusing AxionScriptCore;\n\n";
			scriptFile << "public class Player : Entity {\n";
			scriptFile << "    public override void OnCreate() {\n";
			scriptFile << "        Console.WriteLine(\"Player created!\");\n";
			scriptFile << "    }\n";
			scriptFile << "    public override void OnUpdate(float timestep) {\n";
			scriptFile << "    }\n";
			scriptFile << "}\n";
			scriptFile.close();
		}
	}

}
