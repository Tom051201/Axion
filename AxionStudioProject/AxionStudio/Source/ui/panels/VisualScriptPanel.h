#pragma once

#include <filesystem>
#include <string>
#include <memory>
#include <unordered_map>
#include <map>

#include <Silica/include/SWidget.h>
#include <Silica/include/SNodeEditor.h>

#include "AxionStudio/Source/scripting/VisualScriptGraph.h"

namespace Silica {
	class SBox;
	class SNodeEditor;
}

namespace Axion {

	class VisualScriptPanel {
	public:

		VisualScriptPanel() = default;
		~VisualScriptPanel() = default;

		Silica::WidgetPtr getWidget();

		void setContext(const VisualGraph& graph, const std::filesystem::path& filePath);
		void openScript(const std::filesystem::path& filePath);
		void closeActiveScript();
		void onAssetRenamed(const std::filesystem::path& oldPath, const std::filesystem::path& newPath);
		void onAssetDeleted(const std::filesystem::path& path);

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

		std::unordered_map<int, NodeType> m_nodeTypes;
		std::unordered_map<int, Pin> m_pinMeta;

		std::map<int, Silica::Vec2> m_nodePositions;

		void rebuildUI();
		void rebuildUI_Internal();
		Silica::WidgetPtr buildToolbar();
		Silica::WidgetPtr buildVariablesPanel();
		Silica::WidgetPtr buildNodeContextMenu(Silica::Vec2 mousePos);
		Silica::WidgetPtr buildNodeSpecificContextMenu(Silica::NodeID id, Silica::Vec2 mousePos);

		void spawnNode(NodeType type, Silica::Vec2 canvasPosition);
		void compileAndSave();

		Silica::WidgetPtr createInlineWidgetForPin(const Pin& pin);

		static Silica::Color getNodeTypeColor(NodeType type);
		static Silica::Color getPinColor(PinType type);
		bool hasNodeOfType(NodeType type) const;
		void refreshVariableNodes();
		void syncGraphState();

	};

}
