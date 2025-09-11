#include "EditorStateSerializer.h"
#include "axpch.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

namespace Axion {

	EditorStateSerializer::EditorStateSerializer(const std::string& filepath)
		: m_filepath(filepath) {}

	void EditorStateSerializer::save(const PanelManager& panelManager) {
		YAML::Emitter out;

		out << YAML::BeginMap;
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
