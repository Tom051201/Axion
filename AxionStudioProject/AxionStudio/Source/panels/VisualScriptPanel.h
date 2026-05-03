#pragma once
#include "axpch.h"

#include "AxionStudio/Vendor/ImguiNodeEditor/imgui_node_editor.h"

#include "AxionStudio/Source/core/Panel.h"
#include "AxionStudio/Source/scripting/VisualScriptGraph.h"

namespace Axion {

	class VisualScriptPanel : public Panel {
	public:

		VisualScriptPanel(const std::string& name);
		~VisualScriptPanel() override;

		void setup() override;
		void shutdown() override;
		void onEvent(Event& e) override;
		void onGuiRender() override;

		void setContext(const VisualGraph& graph, const std::filesystem::path& filePath);

	private:

		ax::NodeEditor::EditorContext* m_editorContext = nullptr;
		ax::NodeEditor::Config m_config;
		VisualGraph m_activeGraph;
		std::filesystem::path m_currentFilePath;

		int m_nextLinkId = 1000;
		int m_nextNodeId = 1;
		int m_nextPinId = 10000;

		ImVec2 m_newNodePosition;

		void spawnNode(NodeType type, ImVec2 position);
		bool isPinLinked(int pinId) const;
		bool hasNodeOfType(NodeType type) const;
		void drawToolbar();
		void compileAndSave();

		static ImColor getNodeTypeColor(NodeType type);
		static ImColor getPinColor(PinType type);
		static void drawPinIcon(ImVec2 size, bool connected, PinType type, ImColor color);

	};

}
