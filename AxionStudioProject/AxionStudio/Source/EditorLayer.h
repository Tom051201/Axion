#pragma once

#include "Axion.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/entt/entt.hpp"

#include "AxionStudio/Source/core/EditorCamera.h"
#include "AxionStudio/Source/core/EditorCamera3D.h"
#include "AxionStudio/Source/panels/SystemInfoPanel.h"

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

		EditorCamera m_editorCamera;

		// panels
		Scope<SystemInfoPanel> m_systemInfoPanel;
		bool m_showSystemInfoPanel = false;

		// scene viewport
		Ref<FrameBuffer> m_frameBuffer;
		Vec2 m_viewportDim = { 0.0f, 0.0f };
		bool m_viewportResized = false;
		Ref<Scene> m_activeScene;
		SceneState m_sceneState = SceneState::Editing;

		// ECS
		Entity m_squareEntity;
		Entity m_cameraEntity;

		// ImGui
		ImGuiDockNodeFlags m_dockspaceFlags = 0;
		ImGuiWindowFlags m_windowFlags = 0;

		// TEMP
		Ref<ConstantBuffer> m_tempConstantBuffer;
		Ref<Material> m_tempMaterial;

		bool onWindowResize(WindowResizeEvent& e);
		bool onKeyPressed(KeyPressedEvent& e);

	};

}
