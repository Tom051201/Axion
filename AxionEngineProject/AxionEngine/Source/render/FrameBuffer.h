#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	struct FrameBufferSpecification {
		uint32_t width;
		uint32_t height;
		uint32_t samples = 1;
		Vec4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
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

		virtual const FrameBufferSpecification& getSpecification() const = 0;


		static Ref<FrameBuffer> create(const FrameBufferSpecification& spec);

	};

}
