#pragma once

#include <cstdint>

#include "AxionEngine/Source/core/Ref.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/graphics/Formats.h"

namespace Axion {

	class RenderContext;

	struct FrameBufferSpecification {
		uint32_t width = 1280;
		uint32_t height = 720;
		uint32_t samples = 1;
		Vec4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		ColorFormat textureFormat = ColorFormat::RGBA8;
		DepthStencilFormat depthStencilFormat = DepthStencilFormat::DEPTH32F;
		bool useEntityIDAttachment = false;
	};

	class FrameBuffer : public RefCounted {
	public:

		virtual ~FrameBuffer() = default;

		virtual void release() = 0;
		virtual void resize(uint32_t width, uint32_t height) = 0;

		virtual void bind(RenderContext* renderContext) const = 0;
		virtual void unbind(RenderContext* renderContext) const = 0;

		virtual void clear(RenderContext* renderContext) = 0;
		virtual void clear(RenderContext* renderContext, const Vec4& clearColor) = 0;
		virtual void clearDepth(RenderContext* renderContext) = 0;

		virtual void clearAttachment(RenderContext* renderContext, uint32_t attachmentIndex, int value) = 0;
		virtual int readPixel(RenderContext* renderContext, uint32_t attachmentIndex, int x, int y) = 0;

		virtual void* getColorAttachmentHandle() const = 0;
		virtual void* getColorAttachmentNativeResource() const = 0;
		virtual const FrameBufferSpecification& getSpecification() const = 0;


		static Ref<FrameBuffer> create(const FrameBufferSpecification& spec);

	};

}
