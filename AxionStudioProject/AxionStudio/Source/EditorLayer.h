#pragma once

#include "Axion.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/entt/entt.hpp"

#include "AxionStudio/Source/core/EditorCamera3D.h"
#include "AxionStudio/Source/panels/SystemInfoPanel.h"
#include "AxionStudio/Source/panels/SceneHierarchyPanel.h"
#include "AxionStudio/Source/panels/EditorCameraPanel.h"
#include "AxionStudio/Source/panels/ContentBrowserPanel.h"

namespace Axion {

	enum class SceneState {
		Editing,
		Playing
	};

	class EditorLayer : public Layer {
	public:

		EditorLayer();
		~EditorLayer() = default;

		void onAttach() override;
		void onDetach() override;

		void onUpdate(Axion::Timestep ts) override;
		void onEvent(Axion::Event& e) override;
		void onGuiRender() override;

	private:

		EditorCamera3D m_editorCamera;

		// panels
		Scope<SystemInfoPanel> m_systemInfoPanel;
		Scope<SceneHierarchyPanel> m_sceneHierarchyPanel;
		Scope<EditorCameraPanel> m_editorCameraPanel;
		Scope<ContentBrowserPanel> m_contentBrowserPanel;
		bool m_showSystemInfoPanel = false;
		bool m_showSceneHierarchyPanel = true;
		bool m_showEditorCameraPanel = false;
		bool m_showContentBrowserPanel = true;

		// scene viewport
		Ref<FrameBuffer> m_frameBuffer;
		Vec2 m_viewportDim = { 0.0f, 0.0f };
		bool m_viewportResized = false;
		Ref<Scene> m_activeScene;
		SceneState m_sceneState = SceneState::Editing;

		// ImGui
		ImGuiDockNodeFlags m_dockspaceFlags = 0;
		ImGuiWindowFlags m_windowFlags = 0;

		Entity m_camEntity;

		bool m_newSceneRequested = false;
		bool m_loadSceneRequested = false;

		bool onWindowResize(WindowResizeEvent& e);
		bool onKeyPressed(KeyPressedEvent& e);
		bool onRenderingFinished(RenderingFinishedEvent& e);
	};

}
