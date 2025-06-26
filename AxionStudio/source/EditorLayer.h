#pragma once

#include "Axion.h"

#include "imgui.h"
#include "entt.hpp"

#include "panels/SystemInfoPanel.h"

// TEMP
#include "platform/directx/D12Context.h"

namespace Axion {

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

		OrthographicCameraController m_cameraController;

		Ref<Texture2D> m_texture;

		// panels
		Scope<SystemInfoPanel> m_systemInfoPanel;
		bool m_showSystemInfoPanel = false;

		// scene viewport
		Ref<FrameBuffer> m_frameBuffer;
		Vec2 m_viewportDim = { 0.0f, 0.0f };

		// ECS
		Ref<Scene> m_activeScene;
		Entity m_squareEntity;
		Entity m_cameraEntity;

		// ImGui
		ImGuiDockNodeFlags m_dockspaceFlags = 0;
		ImGuiWindowFlags m_windowFlags = 0;

		// TEMP
		Ref<ConstantBuffer> m_buffer1;
		D12Context* m_context = nullptr;


		bool onWindowResize(WindowResizeEvent& e);

	};

}
