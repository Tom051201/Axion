#include "axpch.h"
#include "OpenGL3Texture.h"

#include "AxionEngine/Vendor/stb_image/stb_image.h"
#include "AxionEngine/Vendor/glad/include/glad/glad.h"

namespace Axion {

	OpenGL3Texture2D::OpenGL3Texture2D(const std::string& path) {
		stbi_set_flip_vertically_on_load(true);

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		AX_CORE_ASSERT(pixels, "Failed to load texture image: {0}", path);

		m_width = static_cast<uint32_t>(texWidth);
		m_height = static_cast<uint32_t>(texHeight);
		m_pixelSize = 4; // RGBA8

		glGenTextures(1, &m_rendererID);
		glBindTexture(GL_TEXTURE_2D, m_rendererID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Or GL_NEAREST
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(pixels);
	}

	OpenGL3Texture2D::~OpenGL3Texture2D() {
		release();
	}

	void OpenGL3Texture2D::release() {
		if (m_rendererID != 0) {
			glDeleteTextures(1, &m_rendererID);
			m_rendererID = 0;
		}
	}

	void OpenGL3Texture2D::bind() const {
		glBindTexture(GL_TEXTURE_2D, m_rendererID);
	}

	void OpenGL3Texture2D::unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void* OpenGL3Texture2D::getHandle() const {
		// TODO : add this function
		return nullptr;
	}

	//-----------------------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------------------//
	//-----------------------------------------------------------------------------------------------------------------------//

	OpenGL3TextureCube::OpenGL3TextureCube(const std::array<std::string, 6>& paths) {
		AX_CORE_ASSERT(false, "OpenGl3TextureCubes are not supported yet!");
	}

	OpenGL3TextureCube::~OpenGL3TextureCube() {
		AX_CORE_ASSERT(false, "OpenGl3TextureCubes are not supported yet!");
	}

	void OpenGL3TextureCube::release() {
		AX_CORE_ASSERT(false, "OpenGl3TextureCubes are not supported yet!");
	}

	void OpenGL3TextureCube::bind() const {
		AX_CORE_ASSERT(false, "OpenGl3TextureCubes are not supported yet!");
	}

	void OpenGL3TextureCube::unbind() const {
		AX_CORE_ASSERT(false, "OpenGl3TextureCubes are not supported yet!");
	}

	void* OpenGL3TextureCube::getHandle() const {
		AX_CORE_ASSERT(false, "OpenGl3TextureCubes are not supported yet!");
		return nullptr;
	}

}
