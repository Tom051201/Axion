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

		// ----- Requesting operations -----
		// Request flags used to defer operations 
		// to once rendering has finished
		enum class RequestFlags : uint32_t {
			None = 0,
			NewScene = 1 << 0,
			OpenScene = 1 << 1,
			SaveScene = 1 << 2,
			SaveSceneAs = 1 << 3,
			NewProject = 1 << 4,
			OpenProject = 1 << 5,
			SaveProject = 1 << 6
		};

		friend constexpr RequestFlags operator|(RequestFlags a, RequestFlags b) {
			return static_cast<RequestFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
		}

		friend constexpr RequestFlags& operator|=(RequestFlags& a, RequestFlags b) {
			a = a | b; return a;
		}

		inline bool hasFlag(RequestFlags flags, RequestFlags check) {
			return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(check)) != 0;
		}

		inline void setFlag(RequestFlags& flags, RequestFlags flag) { flags |= flag; }
		inline void clearFlags(RequestFlags& flags) { flags = RequestFlags::None; }


		// ----- Editor utils -----
		EditorCamera3D m_editorCamera;
		RequestFlags m_requests = RequestFlags::None;


		// ----- Panels -----
		PanelManager m_panelManager;

		SystemInfoPanel* m_systemInfoPanel;
		SceneHierarchyPanel* m_sceneHierarchyPanel;
		EditorCameraPanel* m_editorCameraPanel;
		ContentBrowserPanel* m_contentBrowserPanel;
		ProjectPanel* m_projectPanel;


		// ----- Scene viewport -----
		Ref<FrameBuffer> m_frameBuffer;
		Vec2 m_viewportSize = { 0.0f, 0.0f };
		bool m_viewportResized = false;
		

		// ----- Active scene -----
		Ref<Scene> m_activeScene;
		std::string m_activeSceneFilePath;
		SceneState m_sceneState = SceneState::Editing;


		// ----- Active project -----
		//Ref<Project> m_activeProject;
		std::string m_activeProjectFilePath;


		// ----- ImGui utils -----
		ImGuiDockNodeFlags m_dockspaceFlags = 0;
		ImGuiWindowFlags m_windowFlags = 0;


		// ----- New project window -----
		void drawNewProjectWindow();
		bool m_openNewProjectPopup = false;
		char m_newNameBuffer[128] = "";
		char m_newLocationBuffer[512] = "";


		// ----- Input functions -----
		bool onKeyPressed(KeyPressedEvent& e);
		bool onRenderingFinished(RenderingFinishedEvent& e);


		// ----- Helper functions -----
		void beginDockspace();
		void endDockspace();
		void drawSceneViewport();
		void drawGizmo();
		void drawMenuBar();
		void handleSceneRequests();
		void handleProjectRequests();


		// ----- WIN32 only -----
		#if AX_WIN_USING_CUSTOM_TITLE_BAR
		float m_lastTitleBarMenuX;
		void drawCustomTitleBarWin32();
		#endif
	};

}
