#include "axpch.h"
#include "OpenGL3Shader.h"

#include "AxionEngine/Vendor/glad/include/glad/glad.h"

namespace Axion {

	OpenGL3Shader::OpenGL3Shader() : m_programID(0), m_name("UNSET") {}

	OpenGL3Shader::OpenGL3Shader(const std::string& name) : m_programID(0), m_name(name) {}

	OpenGL3Shader::~OpenGL3Shader() {
		release();
	}

	void OpenGL3Shader::release() {
		if (m_programID != 0) {
			glDeleteProgram(m_programID);
			m_programID = 0;
		}
	}

	void OpenGL3Shader::bind() const {
		glUseProgram(m_programID);
	}

	void OpenGL3Shader::unbind() const {
		glUseProgram(0);
	}

	void OpenGL3Shader::compileFromFile(const std::string& filePath) {
		std::string source = Shader::readShaderFile(filePath);

		auto vsStart = source.find("#type vertex");
		auto psStart = source.find("#type fragment");

		if (vsStart == std::string::npos || psStart == std::string::npos) {
			AX_CORE_LOG_ERROR("Shader file does not contain both vertex and fragment shaders: {0}", filePath);
			return;
		}

		std::string vertexSource = source.substr(vsStart + 12, psStart - (vsStart + 12));
		std::string pixelSource = source.substr(psStart + 14);

		uint32_t vs = compileStage(GL_VERTEX_SHADER, vertexSource);
		uint32_t ps = compileStage(GL_FRAGMENT_SHADER, pixelSource);

		linkProgram(vs, ps);

		glDeleteShader(vs);
		glDeleteShader(ps);

		AX_CORE_LOG_TRACE("Shader '{0}' compiled and linked successfully ({1})", m_name, filePath);
	}

	uint32_t OpenGL3Shader::compileStage(uint32_t type, const std::string& source) {
		uint32_t shader = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(shader, 1, &src, nullptr);
		glCompileShader(shader);

		int success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char infoLog[1024];
			glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
			AX_CORE_LOG_ERROR("Shader Compilation Error ({0}): {1}", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", infoLog);
			glDeleteShader(shader);
			throw std::runtime_error("OpenGL3Shader compilation failed");
		}

		return shader;
	}

	void OpenGL3Shader::linkProgram(uint32_t vertexShader, uint32_t pixelShader) {
		m_programID = glCreateProgram();
		glAttachShader(m_programID, vertexShader);
		glAttachShader(m_programID, pixelShader);
		glLinkProgram(m_programID);

		int success;
		glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[1024];
			glGetProgramInfoLog(m_programID, 1024, nullptr, infoLog);
			AX_CORE_LOG_ERROR("Shader program linking error: {0}", infoLog);
			glDeleteProgram(m_programID);
			throw std::runtime_error("OpenGL3Shader linking failed");
		}

	}

}
