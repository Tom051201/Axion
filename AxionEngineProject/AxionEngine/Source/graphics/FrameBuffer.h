#pragma once

#include <cstdint>

#include "AxionEngine/Source/core/Ref.h"
#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/graphics/Formats.h"

namespace Axion {

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

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void clear() = 0;
		virtual void clear(const Vec4& clearColor) = 0;
		virtual void clearDepth() = 0;

		virtual void clearAttachment(uint32_t attachmentIndex, int value) = 0;
		virtual int readPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void* getColorAttachmentHandle() const = 0;
		virtual void* getColorAttachmentNativeResource() const = 0;
		virtual const FrameBufferSpecification& getSpecification() const = 0;


		static Ref<FrameBuffer> create(const FrameBufferSpecification& spec);

	};

}
