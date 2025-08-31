#pragma once

#include "Axion.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/entt/entt.hpp"

#include "AxionEngine/Source/project/Project.h"

#include "AxionStudio/Source/core/EditorCamera3D.h"
#include "AxionStudio/Source/panels/SystemInfoPanel.h"
#include "AxionStudio/Source/panels/SceneHierarchyPanel.h"
#include "AxionStudio/Source/panels/EditorCameraPanel.h"
#include "AxionStudio/Source/panels/ContentBrowserPanel.h"
#include "AxionStudio/Source/panels/ProjectPanel.h"

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
		Scope<ProjectPanel> m_projectPanel;
		bool m_showSystemInfoPanel = false;
		bool m_showSceneHierarchyPanel = true;
		bool m_showEditorCameraPanel = false;
		bool m_showContentBrowserPanel = true;
		bool m_showProjectPanel = true;

		// scene viewport
		Ref<FrameBuffer> m_frameBuffer;
		Vec2 m_viewportDim = { 0.0f, 0.0f };
		bool m_viewportResized = false;
		
		Ref<Scene> m_activeScene;
		std::string m_activeSceneFilePath;
		SceneState m_sceneState = SceneState::Editing;

		Ref<Project> m_activeProject;
		std::string m_activeProjectFilePath;

		// ImGui
		ImGuiDockNodeFlags m_dockspaceFlags = 0;
		ImGuiWindowFlags m_windowFlags = 0;

		bool m_newSceneRequested = false;
		bool m_openSceneRequested = false;
		bool m_saveSceneRequested = false;
		bool m_saveSceneAsRequested = false;

		bool m_newProjectRequested = false;	 //TODO: review
		bool m_openProjectRequested = false; //TODO: review
		bool m_saveProjectRequested = false; //TODO: review

		bool onKeyPressed(KeyPressedEvent& e);
		bool onRenderingFinished(RenderingFinishedEvent& e);

		void drawNewProjectWindow();
		bool m_showNewProjectWindow = false;
		char m_newNameBuffer[128] = "";
		char m_newLocationBuffer[512] = "";
	};

}
