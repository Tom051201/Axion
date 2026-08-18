#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include <memory>

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
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

#include "AxionStudio/Source/core/EditorCamera.h"
#include "AxionStudio/Source/core/TransformGizmo.h"

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

	enum class EditorState {
		Edit = 0,
		Play,
		Pause,
		Simulate
	};

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
		Ref<Scene> m_editorScene;
		Entity m_selectedEntity;


		// -- Scene --
		Ref<Scene> m_activeScene;
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
		Ref<ContentBrowser> m_contentBrowserPanel;
		Ref<VisualScriptPanel> m_visualScriptPanel;
		Ref<SceneOverviewPanel> m_sceneOverviewPanel;
		Ref<ProjectPanel> m_projectOverviewPanel;
		Ref<AssetManagerPanel> m_assetManagerPanel;
		Ref<HierarchyPanel> m_hierarchyPanel;
		Ref<EntityPropertiesPanel> m_propertiesPanel;
		Ref<ViewportPanel> m_viewportPanel;
		Ref<AssetLibraryPanel> m_assetLibraryPanel;
		Ref<MaterialPanel> m_materialPanel;

		// -- Text Editor Tabs --
		std::unordered_map<std::string, std::string> m_openTextEditors;

		// -- ImGuizmo --
		int m_gizmoType = ImGuizmo::TRANSLATE;

		// -- Gizmo --
		TransformGizmo m_transformGizmo;

		std::shared_ptr<SettingsModal> m_settingsModal;


		void playScene();
		void simScene();
		void stopScene();
		void setSkybox(const std::filesystem::path& path);
		void selectEntity(Entity selectedEntity);
		void newScene();
		void openScene();
		void saveScene();
		void saveSceneAs();
		void drawOverlay();
		void openTextEditorTab(const std::filesystem::path& filepath);
		void openPreferences();

		bool onKeyPressed(KeyPressedEvent& e);

	};

}
