#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Buffers.h"
#include "AxionEngine/Source/render/Texture.h"

namespace Axion {

	class GraphicsContext {
	public:

		virtual ~GraphicsContext() = default;

		virtual void initialize(void* windowHandle, uint32_t width, uint32_t height) = 0;
		virtual void shutdown() = 0;
		virtual void* getNativeContext() const = 0;

		virtual void prepareRendering() = 0;
		virtual void finishRendering() = 0;

		virtual void setClearColor(const Vec4& color) = 0;
		virtual void clear() = 0;

		virtual void bindSwapChainRenderTarget() = 0;
		virtual void* getImGuiTextureID(const Ref<Texture2D>& texture) = 0;

		virtual void resize(uint32_t width, uint32_t height) = 0;

		virtual void activateVsync() = 0;
		virtual void deactivateVsync() = 0;

		virtual void drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib, uint32_t instanceCount = 1) = 0;
		virtual void drawIndexed(const Ref<IndexBuffer>& ib, uint32_t indexCount, uint32_t instanceCount = 1) = 0;

		virtual std::string getGpuName() const = 0;
		virtual std::string getGpuDriverVersion() const = 0;
		virtual uint64_t getVramMB() const = 0;


		static GraphicsContext* get();
		static void set(GraphicsContext* context);
	};

}
