#pragma once

#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/core/Timestep.h"

namespace Axion {

	class Skybox {
	public:

		Skybox(const std::string& crossPath);
		Skybox(const std::array<std::string, 6>& facePaths);
		~Skybox();

		void release();

		void onUpdate(Timestep ts);

		void setTexture(const std::string& crossPath);

		const std::string& getTexturePath() const { return m_texturePath; }

	private:

		Ref<Mesh> m_mesh;
		Ref<TextureCube> m_texture;
		std::string m_texturePath;
		Ref<Shader> m_shader;

		void setupShader(const std::string& filePath);

	};

}
