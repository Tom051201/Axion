#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/GraphicsContext.h"

namespace Axion {

	class OpenGL3Context : public GraphicsContext {
	public:

		~OpenGL3Context() override;

		void initialize(void* hwnd, uint32_t width, uint32_t height) override;
		void shutdown() override;
		void* getNativeContext() const override { return (void*)this; }

		void prepareRendering() override;
		void finishRendering() override;

		void setClearColor(const Vec4& color) override;
		void clear() override;

		void resize(uint32_t width, uint32_t height) override;
		
		void activateVsync() override;
		void deactivateVsync() override;

		void drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) override;

		std::string getGpuName() const override;
		std::string getGpuDriverVersion() const override;
		uint64_t getVramMB() const override;


		// Win32 specific
		#ifdef AX_PLATFORM_WINDOWS
		void setHDC(const HDC& hdc) { m_hdc = hdc; }
		const HDC& getHDC() const { return m_hdc; }
		void setHGLRC(const HGLRC& hglrc) { m_glContext = hglrc; }
		const HGLRC& getHGLRC() const { return m_glContext; }
		#endif

	private:

		uint32_t m_width = 0;
		uint32_t m_height = 0;

		#ifdef AX_PLATFORM_WINDOWS
		
		HWND m_hwnd = nullptr;
		HDC m_hdc = nullptr;
		HGLRC m_glContext = nullptr;

		#endif

	};

}
