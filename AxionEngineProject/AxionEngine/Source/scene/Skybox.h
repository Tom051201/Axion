#pragma once

#include "AxionEngine/Source/core/Ref.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/graphics/Mesh.h"
#include "AxionEngine/Source/graphics/Texture.h"
#include "AxionEngine/Source/graphics/Pipeline.h"

namespace Axion {

	class Skybox : public RefCounted {
	public:

		Skybox(AssetHandle<TextureCube> textureHandle);
		Skybox(AssetHandle<TextureCube> textureHandle, AssetHandle<Pipeline> pipelineHandle);
		~Skybox() = default;

		void release();
		void onUpdate(Timestep ts);

		void setTexture(AssetHandle<TextureCube> textureHandle) { m_textureHandle = textureHandle; }
		AssetHandle<TextureCube> getTextureHandle() const { return m_textureHandle; }

		void setPipeline(AssetHandle<Pipeline> pipelineHandle) { m_pipelineHandle = pipelineHandle; }
		AssetHandle<Pipeline> getPipelineHandle() const { return m_pipelineHandle; }

	private:

		AssetHandle<TextureCube> m_textureHandle;
		AssetHandle<Pipeline> m_pipelineHandle;

	};

}
