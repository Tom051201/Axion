#include "SilicaContext.h"

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

#include "AxionStudio/Source/core/EditorResourceManager.h"

// ----- WINDOWS SPECIFIC INCLUDES --
#ifdef AX_PLATFORM_WINDOWS
#include "AxionStudio/Vendor/Silica/backends/SilicaImplWin32.h"
#endif

// ----- DIRECTX12 SPECIFIC INCLUDES -----
#include "AxionStudio/Vendor/Silica/backends/SilicaImplDX12.h"
#include "AxionEngine/Platform/directx/DX12Context.h"

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
		}

		// ----- SHUTDOWN WINDOWS BACKEND -----
		#ifdef AX_PLATFORM_WINDOWS
		Silica::ImplWin32_shutdown();
		#endif

	}

	void SilicaContext::newFrame() {
		switch (Renderer::getAPI()) {

			// ----- DIRECTX12 IMPLEMENTATION -----
			case RendererAPI::DirectX12: {
				Silica::ImplDX12_newFrame();
				break;
			}

		}
	}

	void SilicaContext::renderDrawData(float width, float height) {
		const Silica::DrawList* drawData = Silica::Renderer::getDrawData();

		switch (Renderer::getAPI()) {

			// ----- DIRECTX12 IMPLEMENTATION -----
			case RendererAPI::DirectX12: {
				auto cmdList = static_cast<DX12Context*>(GraphicsContext::get()->getNativeContext())->getCommandList();
				Silica::ImplDX12_renderDrawData(drawData, cmdList, width, height);
				break;
			}

		}
	}

	void SilicaContext::uploadFontAtlas(Silica::FontAtlas& font) {
		switch (Renderer::getAPI()) {

			// ----- DIRECTX12 IMPLEMENTATION -----
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

			// ----- DIRECTX12 IMPLEMENTATION -----
			case RendererAPI::DirectX12: {
				id = Silica::ImplDX12_registerTexture((ID3D12Resource*)nativeResource);
				break;
			}

		}

		s_textureCache[nativeResource] = id;
		return id;
	}

	Silica::TextureID SilicaContext::getFrameBufferTextureID(const Ref<FrameBuffer>& frameBuffer, Silica::TextureID currentId) {
		if (!frameBuffer) return 0;

		void* nativeResource = frameBuffer->getColorAttachmentNativeResource();
		if (!nativeResource) return 0;

		switch (Renderer::getAPI()) {

			// ----- DIRECTX12 IMPLEMENTATION -----
			case RendererAPI::DirectX12: {
				if (currentId == 0) {
					currentId = Silica::ImplDX12_registerTexture((ID3D12Resource*)nativeResource);
				}
				else {
					Silica::ImplDX12_updateTexture(currentId, (ID3D12Resource*)nativeResource);
				}
				break;
			}

		}

		return currentId;
	}

	Silica::TextureID SilicaContext::getIcon(const std::string& name) {
		Ref<Texture2D> tex = EditorResourceManager::getIcon(name);
		return getTextureID(tex);
	}

}
