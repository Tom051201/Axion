#include "studiopch.h"
#include "SilicaContext.h"

#include <Silica/include/SBox.h>

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"

// ----- WINDOWS SPECIFIC INCLUDES --
#ifdef AX_PLATFORM_WINDOWS
#include <Silica/backends/SilicaImplWin32.h>
#endif

// ----- DIRECTX12 SPECIFIC INCLUDES -----
#include <Silica/backends/SilicaImplDX12.h>
#include "AxionEngine/Platform/directx12/DX12Context.h"

// ----- DIRECTX11 SPECIFIC INCLUDES -----
#include <Silica/backends/SilicaImplDX11.h>
#include "AxionEngine/Platform/directx11/DX11Context.h"

namespace Axion {

	void SilicaContext::initialize() {

		// ----- INIT WINDOWS BACKEND -----
		#ifdef AX_PLATFORM_WINDOWS
		HWND hwnd = (HWND)Application::get().getWindow().getNativeHandle();
		Silica::ImplWin32_init(hwnd);
		#else
		AX_CORE_LOG_FATAL("SilicaContext: Unsupported OS Platform!");
		#endif

		// ----- INIT GRAPHICS BACKEND -----
		switch (Renderer::getAPI()) {
			case RendererAPI::DirectX12: {
				auto dx12Context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
				Silica::ImplDX12_init(dx12Context->getDevice(), 3, DXGI_FORMAT_R8G8B8A8_UNORM);
				break;
			}
			case RendererAPI::DirectX11: {
				auto dx11Context = static_cast<DX11Context*>(GraphicsContext::get()->getNativeContext());
				Silica::ImplDX11_init(dx11Context->getDevice(), dx11Context->getDeviceContext());
				break;
			}
			default: {
				AX_CORE_LOG_FATAL("SilicaContext: Unsupported Renderer API!");
				break;
			}
		}

	}

	void SilicaContext::shutdown() {
		// ----- SHUTDOWN GRAPHICS API -----
		switch (Renderer::getAPI()) {
			case RendererAPI::DirectX12: {
				Silica::ImplDX12_shutdown();
				break;
			}
			case RendererAPI::DirectX11: {
				Silica::ImplDX11_shutdown();
				break;
			}
		}

		// ----- SHUTDOWN WINDOWS BACKEND -----
		#ifdef AX_PLATFORM_WINDOWS
		Silica::ImplWin32_shutdown();
		#endif

	}

	void SilicaContext::newFrame() {
		switch (Renderer::getAPI()) {

			case RendererAPI::DirectX12: {
				Silica::ImplDX12_newFrame();
				break;
			}
			case RendererAPI::DirectX11: {
				Silica::ImplDX11_newFrame();
				break;
			}

		}
	}

	void SilicaContext::renderDrawData(float width, float height) {
		const Silica::DrawList* drawData = Silica::Renderer::getDrawData();

		switch (Renderer::getAPI()) {

			case RendererAPI::DirectX12: {
				auto cmdList = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
				Silica::ImplDX12_renderDrawData(drawData, cmdList, width, height);
				break;
			}
			case RendererAPI::DirectX11: {
				Silica::ImplDX11_renderDrawData(drawData, width, height);
				break;
			}

		}
	}

	void SilicaContext::uploadFontAtlas(Silica::FontAtlas& font) {
		switch (Renderer::getAPI()) {

			case RendererAPI::DirectX12: {
				auto dx12Context = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext());
				ID3D12GraphicsCommandList* cmdList = dx12Context->getCommandList();
				ID3D12CommandAllocator* cmdAlloc = dx12Context->getCommandAllocator();
				ID3D12CommandQueue* cmdQueue = dx12Context->getCommandQueue();

				cmdAlloc->Reset();
				cmdList->Reset(cmdAlloc, nullptr);

				Silica::ImplDX12_uploadFontAtlas(cmdList, font.getPixels(), font.getWidth(), font.getHeight());

				cmdList->Close();
				ID3D12CommandList* ppCommandLists[] = { cmdList };
				cmdQueue->ExecuteCommandLists(1, ppCommandLists);
				dx12Context->waitForPreviousFrame();
				break;
			}
			case RendererAPI::DirectX11: {
				Silica::ImplDX11_uploadFontAtlas(font.getPixels(), font.getWidth(), font.getHeight());
				break;
			}

		}
	}

	void SilicaContext::bindWndProcCallback(std::shared_ptr<Silica::SBox> rootWidget) {
		#ifdef AX_PLATFORM_WINDOWS
		Application::get().getWindow().setWndProcCallback(
			[rootWidget](void* hwnd, unsigned int msg, unsigned long long wparam, long long lparam) {
				return Silica::ImplWin32_wndProcHandler((HWND)hwnd, (UINT)msg, (WPARAM)wparam, (LPARAM)lparam, rootWidget);
			}
		);
		#endif
	}

	void SilicaContext::unbindWndProcCallback() {
		#ifdef AX_PLATFORM_WINDOWS
		Application::get().getWindow().setWndProcCallback(nullptr);
		#endif
	}

	Silica::TextureID SilicaContext::getTextureID(const Ref<Texture2D>& texture) {
		if (!texture) return 0;

		void* nativeResource = texture->getNativeResource();
		if (!nativeResource) return 0;

		if (s_textureCache.find(nativeResource) != s_textureCache.end()) {
			return s_textureCache[nativeResource];
		}

		Silica::TextureID id = 0;
		switch (Renderer::getAPI()) {
			case RendererAPI::DirectX12: id = Silica::ImplDX12_registerTexture((ID3D12Resource*)nativeResource); break;
			case RendererAPI::DirectX11: id = Silica::ImplDX11_registerTexture((ID3D11ShaderResourceView*)texture->getHandle()); break;
		}

		s_textureCache[nativeResource] = id;
		return id;
	}

	Silica::TextureID SilicaContext::getFrameBufferTextureID(const Ref<FrameBuffer>& frameBuffer, Silica::TextureID currentId) {
		if (!frameBuffer) return 0;

		switch (Renderer::getAPI()) {
			case RendererAPI::DirectX12: {
				void* nativeResource = frameBuffer->getColorAttachmentNativeResource();
				if (currentId == 0) return Silica::ImplDX12_registerTexture((ID3D12Resource*)nativeResource);
				else { Silica::ImplDX12_updateTexture(currentId, (ID3D12Resource*)nativeResource); return currentId; }
			}
			case RendererAPI::DirectX11: {
				void* handle = frameBuffer->getColorAttachmentHandle();
				if (currentId == 0) return Silica::ImplDX11_registerTexture((ID3D11ShaderResourceView*)handle);
				else { Silica::ImplDX11_updateTexture(currentId, (ID3D11ShaderResourceView*)handle); return currentId; }
			}
		}
		return 0;
	}

	Silica::TextureID SilicaContext::getIcon(const std::string& name) {
		Ref<Texture2D> tex = EditorResourceManager::getIcon(name);
		return getTextureID(tex);
	}

}
