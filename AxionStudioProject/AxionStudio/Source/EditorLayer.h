#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <memory>

#include <Silica/include/SWidget.h>
#include <Silica/include/FontAtlas.h>
#include <Silica/include/Renderer.h>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/graphics/FrameBuffer.h"
#include "AxionEngine/Source/layers/Layer.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/events/KeyEvent.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/events/MouseEvent.h"

#include "AxionStudio/Source/core/EditorCamera.h"
#include "AxionStudio/Source/core/EditorState.h"
#include "AxionStudio/Source/core/TransformGizmo.h"
#include "AxionStudio/Source/core/EditorEvents.h"

namespace Silica {
	class SBox;
	class SDockSpace;
}

namespace Axion {

	class ContentBrowser;
	class VisualScriptPanel;
	class SceneOverviewPanel;
	class ProjectPanel;
	class AssetManagerPanel;
	class HierarchyPanel;
	class EntityPropertiesPanel;
	class ViewportPanel;
	class AssetLibraryPanel;
	class MaterialPanel;
	class SettingsModal;

	class EditorLayer : public Layer {
	public:

		EditorLayer();
		~EditorLayer() override = default;

		void onAttach() override;
		void onDetach() override;

		void onUpdate(Timestep ts) override;
		void onEvent(Event& e) override;
		void onGuiRender() override;

	private:

		// -- Editor --
		EditorCamera m_editorCamera;
		Vec2 m_viewportSize = { 0.0f, 0.0f };
		Ref<FrameBuffer> m_frameBuffer;
		Shared<Scene> m_editorScene;
		Entity m_selectedEntity;
		int m_hoveredEntityID = -1;


		// -- Scene --
		Shared<Scene> m_activeScene;
		EditorState m_sceneState = EditorState::Edit;
		EditorState m_prePauseState = EditorState::Edit;
		int m_stepFrames = 0;
		std::filesystem::path m_currentScenePath;


		// -- Silica --
		std::shared_ptr<Silica::SBox> m_silicaRoot;
		std::shared_ptr<Silica::SDockSpace> m_dock;
		Silica::WidgetPtr m_mainLayout;
		Silica::FontAtlas m_font;
		Silica::TextureID m_viewportTextureID = 0;
		Shared<ContentBrowser> m_contentBrowserPanel;
		Shared<VisualScriptPanel> m_visualScriptPanel;
		Shared<SceneOverviewPanel> m_sceneOverviewPanel;
		Shared<ProjectPanel> m_projectOverviewPanel;
		Shared<AssetManagerPanel> m_assetManagerPanel;
		Shared<HierarchyPanel> m_hierarchyPanel;
		Shared<EntityPropertiesPanel> m_propertiesPanel;
		Shared<ViewportPanel> m_viewportPanel;
		Shared<AssetLibraryPanel> m_assetLibraryPanel;
		Shared<MaterialPanel> m_materialPanel;
		Shared<SettingsModal> m_settingsModal;

		// -- Text Editor Tabs --
		std::unordered_map<std::string, std::string> m_openTextEditors;

		// -- Gizmo --
		TransformGizmo m_transformGizmo;



		void playScene();
		void simScene();
		void stopScene();
		void newScene();
		void openScene();
		void saveScene();
		void saveSceneAs();
		void drawOverlay();
		void openTextEditorTab(const std::filesystem::path& filepath);
		void openPreferences();
		void openMaterialEditor(const std::filesystem::path& filepath);
		void openVisualScriptPanel(const std::filesystem::path& filepath);
		void openSceneInViewport(const std::filesystem::path& filepath);

		EventReply onKeyPressed(KeyPressedEvent& ev);
		EventReply onKeyReleased(KeyReleasedEvent& ev);
		EventReply onMouseButtonPressed(MouseButtonPressedEvent& ev);
		EventReply onSceneChanged(SceneChangedEvent& ev);
		EventReply onEditorStateChanged(EditorStateChangedEvent& ev);
		EventReply onEntitySelected(EntitySelectedEvent& ev);

	};

}
