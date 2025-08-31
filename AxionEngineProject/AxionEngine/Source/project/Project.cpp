#include "axpch.h"
#include "Project.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include <fstream>

namespace Axion {

	Project::Project(const std::string& name)
		: m_name(name), m_projectPath("Unknown"), m_assetsPath("Unknown"), m_scenesPath("Unknown") {}

	void Project::load(const std::string& path) {
		std::ifstream stream(path);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Project"]) { AX_CORE_LOG_WARN("Loading project file failed"); return; }

		m_name = data["Project"].as<std::string>();
		m_projectPath = path;
		m_assetsPath = data["AssetsPath"].as<std::string>();
		m_scenesPath = data["ScenesPath"].as<std::string>();
	}

	void Project::save(const std::string& path) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value << m_name;
		out << YAML::Key << "ProjectPath" << YAML::Value << m_projectPath;
		out << YAML::Key << "AssetsPath" << YAML::Value << m_assetsPath;
		out << YAML::Key << "ScenesPath" << YAML::Value << m_scenesPath;
		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();
	}

	Ref<Project> Project::createNew(const std::string& location, const std::string& name) {
		namespace fs = std::filesystem;

		Ref<Project> result = std::make_shared<Project>(name);

		try {
			fs::path projectDir = fs::path(location) / name;
			result->setProjectPath(projectDir.string());

			// create project dir
			if (!fs::exists(projectDir)) {
				fs::create_directories(projectDir);
			}

			// create assets dir
			fs::path assetsDir = projectDir / "Assets";
			fs::create_directories(assetsDir);
			result->setAssetsPath(assetsDir.string());

			// create scenes dir
			fs::path scenesDir = projectDir / "Scenes";
			fs::create_directories(scenesDir);
			result->setScenesPath(scenesDir.string());

			// create project file (.axproj)
			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Project" << YAML::Value << name;
			out << YAML::Key << "ProjectPath" << YAML::Value << result->getProjectPath();
			out << YAML::Key << "AssetsPath" << YAML::Value << result->getAssetsPath();
			out << YAML::Key << "ScenesPath" << YAML::Value << result->getScenesPath();
			out << YAML::EndMap;

			std::ofstream fout(projectDir / (name + ".axproj"));
			fout << out.c_str();

			return result;
		}
		catch (std::exception& e) {
			AX_CORE_LOG_ERROR("Failed to create project: {}", e.what());
			return nullptr;
		}
	}

}
