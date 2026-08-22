#pragma once

#include <array>
#include <cstdint>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

#include "AxionEngine/Source/core/Ref.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"
#include "AxionEngine/Source/graphics/Texture.h"

namespace Axion {

	class DX11Context : public GraphicsContext {
	public:

		~DX11Context() override;

		void initialize(void* hwnd, uint32_t width, uint32_t height) override;
		void shutdown() override;
		void* getNativeContext() const override { return (void*)this; }

		void prepareRendering() override;
		void finishRendering() override;

		void setClearColor(const Vec4& color) override;
		void clear() override;

		void bindSwapChainRenderTarget() override;
		void bindDepthOnlyRenderTarget(const Ref<Texture2D>& depthTexture) override;
		void unbindDepthOnlyRenderTarget(const Ref<Texture2D>& depthTexture) override;
		void* getImGuiTextureID(const Ref<Texture2D>& texture) override; // TODO: maybe remove this imgui thing
		void bindSrvTable(uint32_t rootIndex, const std::array<Ref<Texture2D>, 16>& textures, uint32_t count);

		void resize(uint32_t width, uint32_t height) override;

		void activateVsync() override { m_vsyncInterval = 1; }
		void deactivateVsync() override { m_vsyncInterval = 0; }

		void drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, uint32_t instanceCount = 1) override;
		void drawIndexed(const Ref<IndexBuffer>& ib, uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0) override;
		void draw(uint32_t vertexCount) override;

		std::string getGpuName() const override;
		std::string getGpuDriverVersion() const override;
		uint64_t getVramMB() const override;

		// ----- DX11 Specific Getters -----
		ID3D11Device* getDevice() const { return m_device.Get(); }
		ID3D11DeviceContext* getDeviceContext() const { return m_deviceContext.Get(); }

	private:

		uint32_t m_width = 0, m_height = 0;
		uint32_t m_vsyncInterval = 0;
		Vec4 m_clearColor = Vec4::zero();

		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferRTV;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

		void createRenderTarget();
		void cleanupRenderTarget();

	};

}
