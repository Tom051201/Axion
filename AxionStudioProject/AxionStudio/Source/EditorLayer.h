#pragma once

#include "Axion.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/ImGuizmo/ImGuizmo.h"

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SImage.h"
#include "AxionStudio/Vendor/Silica/include/SDockspace.h"

#include "AxionStudio/Source/core/EditorCamera.h"

namespace Axion {

	class ContentBrowser;
	class VisualScriptPanel;
	class SceneOverviewPanel;
	class ProjectPanel;
	class AssetManagerPanel;
	class HierarchyPanel;
	class EntityPropertiesPanel;
	class ViewportPanel;

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


		// -- ImGuizmo --
		int m_gizmoType = ImGuizmo::TRANSLATE;


		void playScene();
		void simScene();
		void stopScene();
		void selectEntity(Entity selectedEntity);
		void drawOverlay();

	};

}
