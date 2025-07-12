#include "axpch.h"
#include "OpenGL3FrameBuffer.h"

#include "AxionEngine/Vendor/glad/include/glad/glad.h"

namespace Axion {

	OpenGL3FrameBuffer::OpenGL3FrameBuffer(const FrameBufferSpecification& spec)
		: m_specification(spec) {
		resize(spec.width, spec.height);
	}

	OpenGL3FrameBuffer::~OpenGL3FrameBuffer() {
		release();
	}

	void OpenGL3FrameBuffer::release() {
		glDeleteFramebuffers(1, &m_rendererID);
		glDeleteTextures(1, &m_colorAttachment);
		glDeleteTextures(1, &m_depthAttachment);
	}

	void OpenGL3FrameBuffer::resize(uint32_t width, uint32_t height) {
		width = std::max(1u, width);
		height = std::max(1u, height);

		m_specification.width = width;
		m_specification.height = height;

		if (m_rendererID) release();

		glGenFramebuffers(1, &m_rendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);

		// color attachment
		glGenTextures(1, &m_colorAttachment);
		glBindTexture(GL_TEXTURE_2D, m_colorAttachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorAttachment, 0);

		// depth attachment
		glGenTextures(1, &m_depthAttachment);
		glBindTexture(GL_TEXTURE_2D, m_depthAttachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment, 0);

		AX_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGL3FrameBuffer::bind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, m_rendererID);
		glViewport(0, 0, m_specification.width, m_specification.height);
	}

	void OpenGL3FrameBuffer::unbind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGL3FrameBuffer::clear() {
		clear(m_specification.clearColor);
	}

	void OpenGL3FrameBuffer::clear(const Vec4& clearColor) {
		bind();
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void* OpenGL3FrameBuffer::getColorAttachmentHandle() const {
		return reinterpret_cast<void*>(static_cast<uintptr_t>(m_colorAttachment));
	}

}
