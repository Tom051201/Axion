#pragma once

#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/render/Pipeline.h"
#include "AxionEngine/Source/core/Timestep.h"

namespace Axion {

	class Skybox {
	public:

		Skybox(const std::string& crossPath, const std::string& pipelinePath);
		Skybox(const std::array<std::string, 6>& facePaths, const std::string& pipelinePath);
		~Skybox();

		void release();

		void onUpdate(Timestep ts);

		void setTexture(const std::string& crossPath);

		const std::string& getTexturePath() const { return m_texturePath; }

	private:

		Ref<Mesh> m_mesh;
		Ref<TextureCube> m_texture;
		std::string m_texturePath;
		AssetHandle<Pipeline> m_pipelineHandle;

		void setupPipeline(const std::string& filePath);

	};

}
