#pragma once

#include "Axion.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/entt/entt.hpp"

#include "AxionEngine/Source/project/Project.h"

#include "AxionStudio/Source/core/EditorCamera3D.h"
#include "AxionStudio/Source/core/PanelManager.h"
#include "AxionStudio/Source/panels/SystemInfoPanel.h"
#include "AxionStudio/Source/panels/SceneHierarchyPanel.h"
#include "AxionStudio/Source/panels/EditorCameraPanel.h"
#include "AxionStudio/Source/panels/ContentBrowserPanel.h"
#include "AxionStudio/Source/panels/ProjectPanel.h"
#include "AxionStudio/Source/panels/SceneOverviewPanel.h"

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

		void onUpdate(Timestep ts) override;
		void onEvent(Event& e) override;
		void onGuiRender() override;

	private:

		// ----- Editor utils -----
		EditorCamera3D m_editorCamera;


		// ----- Panels -----
		PanelManager m_panelManager;

		SystemInfoPanel* m_systemInfoPanel;
		SceneHierarchyPanel* m_sceneHierarchyPanel;
		EditorCameraPanel* m_editorCameraPanel;
		ContentBrowserPanel* m_contentBrowserPanel;
		ProjectPanel* m_projectPanel;
		SceneOverviewPanel* m_sceneOverviewPanel;


		// ----- Scene viewport -----
		Ref<FrameBuffer> m_frameBuffer;
		Vec2 m_viewportSize = { 0.0f, 0.0f };
		bool m_viewportResized = false;
		

		// ----- Active scene -----
		Ref<Scene> m_activeScene;
		std::string m_activeSceneFilePath;
		SceneState m_sceneState = SceneState::Editing;


		// ----- Active project -----
		std::string m_activeProjectFilePath;


		// ----- ImGui utils -----
		ImGuiDockNodeFlags m_dockspaceFlags = 0;
		ImGuiWindowFlags m_windowFlags = 0;


		// ----- New project window -----
		void drawNewProjectWindow();
		bool m_openNewProjectPopup = false;
		char m_newNameBuffer[128] = "";
		char m_newLocationBuffer[512] = "";
		char m_newProjectAuthor[128] = "";
		char m_newProjectCompany[128] = "";
		char m_newProjectDescription[128] = "";


		// ----- Input functions -----
		bool onKeyPressed(KeyPressedEvent& e);
		bool onRenderingFinished(RenderingFinishedEvent& e);
		bool onSceneChanged(SceneChangedEvent& e);


		// ----- Helper functions -----
		void beginDockspace();
		void endDockspace();
		void drawSceneViewport();
		void drawGizmo();
		void drawMenuBar();


		// ----- WIN32 only -----
		#if AX_WIN_USING_CUSTOM_TITLE_BAR
		float m_lastTitleBarMenuX;
		void drawCustomTitleBarWin32();
		#endif
	};

}
