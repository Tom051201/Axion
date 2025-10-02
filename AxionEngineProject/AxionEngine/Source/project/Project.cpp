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
		: m_name(name), m_projectPath("Unknown"), m_assetsPath("Unknown"), m_scenesPath("Unknown") {}

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
		if (data["ScenesPath"]) project->setScenesPath(data["ScenesPath"].as<std::string>());

		// ----- Default scene -----
		if (data["DefaultScene"]) project->setDefaultScene(data["DefaultScene"].as<std::string>());

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
		out << YAML::Key << "ScenesPath" << YAML::Value << m_scenesPath;

		if (!m_defaultScene.empty()) out << YAML::Key << "DefaultScene" << YAML::Value << m_defaultScene;

		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();
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

			// create scenes dir
			fs::path scenesDir = projectDir / "Scenes";
			fs::create_directories(scenesDir);

			// setup project
			result->setName(spec.name);
			result->setVersion("0.1.0");
			result->setEngineVersion(AX_ENGINE_VERSION);
			result->setProjectPath(projectDir.string());
			result->setScenesPath(scenesDir.string());
			result->setAssetsPath(assetsDir.string());

			if (!spec.author.empty()) result->setAuthor(spec.author);
			if (!spec.company.empty()) result->setCompany(spec.company);
			if (!spec.description.empty()) result->setDescription(spec.description);

			// write the project file (.axproj)
			result->save((projectDir / (projectName + ".axproj")).string());

			return result;
		}
		catch (std::exception& e) {
			AX_CORE_LOG_ERROR("Failed to create project: {}", e.what());
			(void)e;
			return nullptr;
		}
	}

}
