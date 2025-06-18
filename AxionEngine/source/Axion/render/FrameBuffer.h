#pragma once

#include "Axion/core/Core.h"
#include "Axion/core/Math.h"

namespace Axion {

	struct FrameBufferSpecification {
		uint32_t width;
		uint32_t height;
		uint32_t samples = 1;
	};

	class FrameBuffer {
	public:

		virtual ~FrameBuffer() = default;

		virtual void release() = 0;
		virtual void resize() = 0;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		virtual void clear(const Vec4& clearColor) = 0;

		virtual const FrameBufferSpecification& getSpecification() const = 0;


		static Ref<FrameBuffer> create(const FrameBufferSpecification& spec);

	};

}
