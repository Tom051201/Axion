#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/render/Pipeline.h"
#include "AxionEngine/Source/render/Buffers.h"
#include "AxionEngine/Source/render/MaterialData.h"
#include "AxionEngine/Source/render/Texture.h"

namespace Axion {

	class Material {
	public:

		Material(const std::string& name, const AssetHandle<Pipeline>& pipelineHandle, const MaterialProperties& properties);
		~Material();

		void release();

		void bind();
		void unbind();

		bool isValid() const;

		void setAlbedoColor(const Vec4& color) { m_properties.albedoColor = color; m_dirty = true; }
		Vec4 getAlbedoColor() const { return m_properties.albedoColor; }
		void setMetalness(float value) { m_properties.metalness = value; m_dirty = true; }
		float getMetalness() const { return m_properties.metalness; }
		void setRoughness(float value) { m_properties.roughness = value; m_dirty = true; }
		float getRoughness() const { return m_properties.roughness; }
		void setEmission(float value) { m_properties.emissionStrength = value; m_dirty = true; }
		float getEmission() const { return m_properties.emissionStrength; }

		void setTexture(TextureSlot slot, const AssetHandle<Texture2D>& texture);
		AssetHandle<Texture2D> getTexture(TextureSlot slot) const;

		const std::string& getName() const { return m_name; }
		AssetHandle<Pipeline> getPipelineHandle() const { return m_pipelineHandle; }

		static Ref<Material> create(const std::string& name, const AssetHandle<Pipeline>& pipelineHandle, const MaterialProperties& properties = {});

	private:

		std::string m_name = "Unknown Material";
		AssetHandle<Pipeline> m_pipelineHandle;

		MaterialProperties m_properties;

		Ref<ConstantBuffer> m_materialBuffer;

		std::map<TextureSlot, AssetHandle<Texture2D>> m_textures; // TODO: make this an array

		mutable bool m_dirty = true;

		bool m_slotsCached = false;
		int m_materialBufferSlot = -1;
		uint32_t m_textureTableSlot = 0;
		std::map<TextureSlot, int> m_textureIndices;

	};

}
