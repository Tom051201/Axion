#include "EditorStateSerializer.h"
#include "axpch.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorConfig.h"

namespace Axion {

	EditorStateSerializer::EditorStateSerializer(const std::string& filepath)
		: m_filepath(filepath) {}

	void EditorStateSerializer::save(const PanelManager& panelManager) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		// -- Startup project --
		out << YAML::Key << "StartupProject" << YAML::Value << EditorConfig::startupProjectPath;

		// -- Panels --
		out << YAML::Key << "Panels" << YAML::Value << YAML::BeginSeq;
		for (const auto& panel : panelManager.getAllPanels()) {
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << panel->getName();
			out << YAML::Key << "Visible" << YAML::Value << panel->isVisible();
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::ofstream fout(m_filepath);
		fout << out.c_str();
	}

	void EditorStateSerializer::load(PanelManager& panelManager) {
		if (!std::filesystem::exists(m_filepath)) {
			AX_CORE_LOG_ERROR("Failed to laod Editor State: File does not exist");
			return;
		}

		YAML::Node data = YAML::LoadFile(m_filepath);

		if (data["StartupProject"]) {
			std::string path = data["StartupProject"].as<std::string>();
			if (path != "None") {
				EditorConfig::startupProjectPath = path;

				Ref<Project> project = std::make_shared<Project>("");
				project->load(path);
				ProjectManager::setActiveProject(project);
			}
		}

		if (data["Panels"]) {
			for (auto panelNode : data["Panels"]) {
				std::string name = panelNode["Name"].as<std::string>();
				bool visible = panelNode["Visible"].as<bool>();

				if (auto* panel = panelManager.findPanelByName(name)) {
					panel->isVisible() = visible;
				}
			}
		}
	}

}
