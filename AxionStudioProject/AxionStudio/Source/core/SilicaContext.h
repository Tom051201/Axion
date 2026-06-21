#pragma once

#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/render/FrameBuffer.h"

#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"

namespace Axion {

	class SilicaContext {
	public:

		static void initialize();
		static void shutdown();

		static void newFrame();
		static void renderDrawData(float width, float height);

		static void uploadFontAtlas(Silica::FontAtlas& font);
		static void bindWndProcCallback(std::shared_ptr<Silica::SBox> rootWidget);
		static void unbindWndProcCallback();

		static Silica::TextureID getTextureID(const Ref<Texture2D>& texture);
		static Silica::TextureID getFrameBufferTextureID(const Ref<FrameBuffer>& frameBuffer, Silica::TextureID currentId = 0);
		static Silica::TextureID getIcon(const std::string& name);

	private:

		inline static std::unordered_map<void*, Silica::TextureID> s_textureCache;

	};

}
