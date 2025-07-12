#include "axpch.h"
#include "OpenGL3Context.h"

#include "AxionEngine/Source/core/Application.h"

#include "AxionEngine/Vendor/glad/include/glad/glad.h"
#include "AxionEngine/Vendor/glad/include/glad/glad_wgl.h"

namespace Axion {

	OpenGL3Context::~OpenGL3Context() {}

	void OpenGL3Context::initialize(void* hwnd, uint32_t width, uint32_t height) {
		AX_ASSERT(hwnd, "HWND cannot be null");
		m_width = width;
		m_height = height;

		m_hwnd = static_cast<HWND>(hwnd);
		m_hdc = GetDC(m_hwnd);

		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			32,
			0, 0, 0, 0, 0, 0,
			0,
			0,
			24,
			8,
			0,
			PFD_MAIN_PLANE,
			0, 0, 0, 0
		};
		int pixelformat = ChoosePixelFormat(m_hdc, &pfd);
		SetPixelFormat(m_hdc, pixelformat, &pfd);

		HGLRC tempCtx = wglCreateContext(m_hdc);
		wglMakeCurrent(m_hdc, tempCtx);
		
		// Load wgl extensions
		auto wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

		int attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 5,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		m_glContext = wglCreateContextAttribsARB(m_hdc, 0, attribs);
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(tempCtx);

		wglMakeCurrent(m_hdc, m_glContext);

		if (!gladLoadGL()) {
			Application::get().closeOnError("Failed to load OpenGL functions! Application had to shutdown!");
		}
		if (!gladLoadWGL(m_hdc)) {
			Application::get().closeOnError("Failed to load WGL! Application had to shutdown!");
		}

		AX_CORE_LOG_INFO("OpenGL backend initialized successfully");
	}

	void OpenGL3Context::shutdown() {
		wglDeleteContext(m_glContext);
		ReleaseDC(m_hwnd, m_hdc);

		AX_CORE_LOG_INFO("OpenGL backend shutdown");
	}

	void OpenGL3Context::prepareRendering() {
	
	}

	void OpenGL3Context::finishRendering() {
	
	}

	void OpenGL3Context::setClearColor(const Vec4& color) {
		glClearColor(color.x / 255, color.y / 255, color.z / 255, color.w / 255);
	}

	void OpenGL3Context::clear() {
		glClear(GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT*/);
	}

	void OpenGL3Context::resize(uint32_t width, uint32_t height) {
		
	}

	void OpenGL3Context::drawIndexed(const Ref<VertexBuffer>& vb, const Ref<IndexBuffer>& ib) {

	}

	void OpenGL3Context::activateVsync() {
		if (wglSwapIntervalEXT) {
			wglSwapIntervalEXT(1);
		}
	}

	void OpenGL3Context::deactivateVsync() {
		if (wglSwapIntervalEXT) {
			wglSwapIntervalEXT(0);
		}
	}

	std::string OpenGL3Context::getGpuName() const {
		const GLubyte* renderer = glGetString(GL_RENDERER);
		if (renderer) {
			return reinterpret_cast<const char*>(renderer);
		}
		else {
			return "Unknown";
		}
	}

	std::string OpenGL3Context::getGpuDriverVersion() const {
		const GLubyte* version = glGetString(GL_VERSION);
		if (version) {
			return reinterpret_cast<const char*>(version);
		}
		else {
			return "Unknown";
		}
	}

	uint64_t OpenGL3Context::getVramMB() const {
		GLint memKB = 0;

		if (GLAD_GL_NVX_gpu_memory_info) {
			glGetIntegerv(0x9048, &memKB);
		}
		else if (GLAD_GL_ATI_meminfo) {
			GLint vboStats[4];
			glGetIntegerv(0x87FB, vboStats);
			memKB = vboStats[0];
		}

		return static_cast<uint64_t>(memKB) / 1024;
	}

}
