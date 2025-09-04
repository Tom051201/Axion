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


	private:

		Ref<Mesh> m_mesh;
		Ref<TextureCube> m_texture;
		Ref<Shader> m_shader;

	};

}
