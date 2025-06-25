#pragma once

#include "Axion.h"

#include "imgui.h"

// TEMP
#include "platform/directx/D12Context.h"

namespace Axion {

	struct SystemInfo {
		std::string gpuName = "?";
		std::string gpuDriverVersion = "?";
		uint64_t vramMB = 0;

		std::string cpuName = "?";
		uint32_t cores = 0;

		uint64_t totalRamMB = 0;
		std::string os = "?";
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

		SystemInfo& getSystemInfo() { return m_systemInfo; }
		const SystemInfo& getSystemInfo() const { return m_systemInfo; }

	private:

		OrthographicCameraController m_cameraController;

		Ref<Texture2D> m_texture;

		Ref<FrameBuffer> m_frameBuffer;
		Vec2 m_viewportDim = { 0.0f, 0.0f };

		static bool s_isDragging;
		static POINT s_dragOffset;
		bool m_showSystemInfoWindow = false;

		// ImGui
		ImGuiDockNodeFlags m_dockspaceFlags;
		ImGuiWindowFlags m_windowFlags;

		// TEMP
		Ref<ConstantBuffer> m_buffer1;
		D12Context* m_context = nullptr;
		float m_testColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		ImTextureID m_imGuiTexID = 0;

		SystemInfo m_systemInfo;

		bool onWindowResize(WindowResizeEvent& e);

		void setupSystemInfo();

	};

}
