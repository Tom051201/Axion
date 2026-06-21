#pragma once

#include "axpch.h"
#include "AxionStudio/Source/scripting/VisualScriptGraph.h"

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"
#include "AxionStudio/Vendor/Silica/include/SNodeEditor.h"

#include <unordered_map>
#include <functional>

namespace Axion {

	class VisualScriptPanel {
	public:

		VisualScriptPanel() = default;
		~VisualScriptPanel() = default;

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font);

		void setContext(const VisualGraph& graph, const std::filesystem::path& filePath);
		void openScript(const std::filesystem::path& filePath);
		void closeActiveScript();

		const std::filesystem::path& getActiveFilePath() const { return m_currentFilePath; }

	private:
		VisualGraph m_activeGraph;
		std::filesystem::path m_currentFilePath;
		std::string m_currentLayoutFilePath;

		int m_nextLinkId = 1000;
		int m_nextNodeId = 1;
		int m_nextPinId = 10000;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		std::shared_ptr<Silica::SNodeEditor> m_nodeEditor;

		Silica::FontAtlas* m_font = nullptr;

		std::unordered_map<int, NodeType> m_nodeTypes;
		std::unordered_map<int, Pin> m_pinMeta;

		void rebuildUI();
		void rebuildUI_Internal();
		Silica::WidgetPtr buildToolbar();
		Silica::WidgetPtr buildVariablesPanel();
		Silica::WidgetPtr buildNodeContextMenu(Silica::Vec2 mousePos);

		void spawnNode(NodeType type, Silica::Vec2 canvasPosition);
		void compileAndSave();

		static Silica::Color getNodeTypeColor(NodeType type);
		static Silica::Color getPinColor(PinType type);

	};

}
