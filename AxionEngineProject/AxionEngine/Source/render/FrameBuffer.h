#pragma once

#include "axpch.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	enum class FrameBufferTextureFormat {
		None = 0,
		RGBA8,
		RED_INTEGER,
		RGBA16F,
		BGRA8,
		RGB10A2
	};

	enum class FrameBufferDepthFormat {
		None = 0,
		DEPTH24_STENCIL8,
		DEPTH32F,
		DEPTH32F_STENCIL8,
		DEPTH16
	};

	struct FrameBufferSpecification {
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t samples = 1;
		Vec4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		FrameBufferTextureFormat textureFormat = FrameBufferTextureFormat::None;
		FrameBufferDepthFormat depthFormat = FrameBufferDepthFormat::None;
	};

	class FrameBuffer {
	public:

		virtual ~FrameBuffer() = default;

		virtual void release() = 0;
		virtual void resize(uint32_t width, uint32_t height) = 0;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void clear() = 0;
		virtual void clear(const Vec4& clearColor) = 0;

		virtual void* getColorAttachmentHandle() const = 0;
		virtual const FrameBufferSpecification& getSpecification() const = 0;


		static Ref<FrameBuffer> create(const FrameBufferSpecification& spec);

	};

}
