#pragma once

#include "Axion.h"

#include "imgui.h"

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

		Ref<FrameBuffer> m_frameBuffer;
		Vec2 m_viewportDim = { 0.0f, 0.0f };

		static bool s_isDragging;
		static POINT s_dragOffset;

		// ImGui
		ImGuiDockNodeFlags m_dockspaceFlags;
		ImGuiWindowFlags m_windowFlags;

		// TEMP
		Ref<ConstantBuffer> m_buffer1;
		D12Context* m_context = nullptr;
		float m_testColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };



		bool onWindowResize(WindowResizeEvent& e);

	};

}
