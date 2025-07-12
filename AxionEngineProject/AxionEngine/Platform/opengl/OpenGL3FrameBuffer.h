#pragma once

#include "AxionEngine/Source/render/FrameBuffer.h"

namespace Axion {

	class OpenGL3FrameBuffer : public FrameBuffer {
	public:

		OpenGL3FrameBuffer(const FrameBufferSpecification& spec);
		~OpenGL3FrameBuffer() override;

		void release() override;
		void resize(uint32_t width, uint32_t height) override;

		void bind() const override;
		void unbind() const override;

		void clear() override;
		void clear(const Vec4& clearColor) override;

		const FrameBufferSpecification& getSpecification() const override { return m_specification; }

		uint32_t getColorAttachmentRendererID() const { return m_colorAttachment; }
		uint32_t getDepthAttachmentRendererID() const { return m_depthAttachment; }

	private:

		FrameBufferSpecification m_specification;

		uint32_t m_rendererID = 0;
		uint32_t m_colorAttachment = 0;
		uint32_t m_depthAttachment = 0;

	};

}
