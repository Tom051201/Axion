#pragma once

#include "AxionEngine/Source/render/Texture.h"

namespace Axion {

	class OpenGL3Texture2D : public Texture2D {
	public:

		OpenGL3Texture2D(const std::string& path);
		OpenGL3Texture2D(uint32_t width, uint32_t height, void* data);
		~OpenGL3Texture2D() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		uint32_t getWidth() const override { return m_width; }
		uint32_t getHeight() const override { return m_height; }

		void* getHandle() const override;

	private:

		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_pixelSize = 4;

		uint32_t m_rendererID = 0;

	};



	class OpenGL3TextureCube : public TextureCube {
	public:

		OpenGL3TextureCube(const std::string& filePath);
		OpenGL3TextureCube(const std::array<std::string, 6>& paths);
		~OpenGL3TextureCube() override;

		void release() override;

		void bind() const override;
		void unbind() const override;

		uint32_t getFaceWidth() const override { AX_CORE_ASSERT(false, "OpenGl3TextureCubes are not supported yet!"); return 0; }
		uint32_t getFaceHeight() const override { AX_CORE_ASSERT(false, "OpenGl3TextureCubes are not supported yet!"); return 0; }

		void* getHandle() const override;

	private:

	};

}
