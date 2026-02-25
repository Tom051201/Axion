#pragma once

#include "Axion.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/entt/entt.hpp"

#include "AxionEngine/Source/project/Project.h"

#include "AxionStudio/Source/core/EditorCamera.h"
#include "AxionStudio/Source/core/PanelManager.h"
#include "AxionStudio/Source/core/ModalManager.h"
#include "AxionStudio/Source/panels/SystemInfoPanel.h"
#include "AxionStudio/Source/panels/SceneHierarchyPanel.h"
#include "AxionStudio/Source/panels/EditorCameraPanel.h"
#include "AxionStudio/Source/panels/ContentBrowserPanel.h"
#include "AxionStudio/Source/panels/ProjectPanel.h"
#include "AxionStudio/Source/panels/SceneOverviewPanel.h"
#include "AxionStudio/Source/panels/AssetManagerPanel.h"
#include "AxionStudio/Source/modals/SkyboxImportModal.h"
#include "AxionStudio/Source/modals/MeshImportModal.h"
#include "AxionStudio/Source/modals/AudioImportModal.h"
#include "AxionStudio/Source/modals/ShaderImportModal.h"
#include "AxionStudio/Source/modals/MaterialImportModal.h"
#include "AxionStudio/Source/modals/Texture2DImportModal.h"
#include "AxionStudio/Source/modals/PipelineImportModal.h"

namespace Axion {

	enum class SceneState {
		Edit = 0,
		Play,
		Pause,
		Simulate
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
		EditorCamera m_editorCamera;


		// ----- Panels -----
		PanelManager m_panelManager;

		SystemInfoPanel* m_systemInfoPanel;
		SceneHierarchyPanel* m_sceneHierarchyPanel;
		EditorCameraPanel* m_editorCameraPanel;
		ContentBrowserPanel* m_contentBrowserPanel;
		ProjectPanel* m_projectPanel;
		SceneOverviewPanel* m_sceneOverviewPanel;
		AssetManagerPanel* m_assetManagerPanel;


		// ----- Modals -----
		ModalManager m_modalManager;

		SkyboxImportModal* m_skyboxImportModal;
		MeshImportModal* m_meshImportModal;
		AudioImportModal* m_audioImportModal;
		ShaderImportModal* m_shaderImportModal;
		MaterialImportModal* m_materialImportModal;
		Texture2DImportModal* m_tex2dImportModal;
		PipelineImportModal* m_pipelineImportModal;


		// ----- Scene viewport -----
		Ref<FrameBuffer> m_frameBuffer;
		Vec2 m_viewportSize = { 0.0f, 0.0f };
		bool m_viewportResized = false;


		// ----- Active scene -----
		Ref<Scene> m_activeScene;
		Ref<Scene> m_editorScene;
		std::string m_activeSceneFilePath;
		SceneState m_sceneState = SceneState::Edit;
		SceneState m_prePauseState = SceneState::Edit;
		int m_stepFrames = 0;


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


		// ----- Event functions -----
		bool onKeyPressed(KeyPressedEvent& e);
		bool onRenderingFinished(RenderingFinishedEvent& e);
		bool onSceneChanged(SceneChangedEvent& e);
		bool onFileDrop(FileDropEvent& e);


		// ----- Helper functions -----
		void beginDockspace();
		void endDockspace();
		void drawSceneViewport();
		void drawGizmo();
		void drawMenuBar();
		void drawToolBar();

		void onScenePlay();
		void onSceneSimulate();
		void onSceneStop();

		void drawOverlay();
		void drawWireframeBox(const Mat4& transform, const Vec4& color);
	};

}
