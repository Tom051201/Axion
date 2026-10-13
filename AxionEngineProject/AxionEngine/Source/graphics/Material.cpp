#include "axpch.h"
#include "Material.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/graphics/Renderer.h"

namespace Axion {

	Material::Material(const std::string& name, const MaterialProperties& properties)
		: m_name(name), m_properties(properties) {
		m_materialBuffer = ConstantBuffer::create(sizeof(MaterialProperties));
	}

	Material::Material(const std::string& name, const AssetHandle<Pipeline>& pipelineHandle, const MaterialProperties& properties)
		: m_name(name), m_pipelineHandle(pipelineHandle), m_properties(properties) {
		m_materialBuffer = ConstantBuffer::create(sizeof(MaterialProperties));
	}

	Material::~Material() {
		release();
	}

	void Material::release() {
		m_materialBuffer->release();
	}

	void Material::bind(RenderContext* renderContext) {
		Ref<Pipeline> pipeline;
		if (m_pipelineHandle.isValid()) pipeline = AssetManager::get<Pipeline>(m_pipelineHandle);
		else pipeline = m_isSkeletal ? EngineAssets::getSkeletalPBRPipeline() : EngineAssets::getStandardPBRPipeline();
		if (!pipeline) return;

		pipeline->bind(renderContext);

		if (m_dirty) {
			m_materialBuffer->update(&m_properties, sizeof(MaterialProperties));
			m_dirty = false;
		}

		m_materialBuffer->bind(renderContext, (uint32_t)RootSlot::Material);
	}

	void Material::unbind(RenderContext* renderContext) {}

	bool Material::isValid() const {
		if (m_pipelineHandle.isValid()) {
			return AssetManager::get<Pipeline>(m_pipelineHandle) != nullptr;
		}
		return EngineAssets::getStandardPBRPipeline() != nullptr;
	}

	void Material::setTexture(TextureSlot slot, const AssetHandle<Texture2D>& texture) {
		m_textures[slot] = texture;

		switch (slot) {
			case TextureSlot::Normal: { m_properties.useNormalMap = texture.isValid() ? 1.0f : 0.0f; break; }
			case TextureSlot::Metalness: { m_properties.useMetalnessMap = texture.isValid() ? 1.0f : 0.0f; break; }
			case TextureSlot::Roughness: { m_properties.useRoughnessMap = texture.isValid() ? 1.0f : 0.0f; break; }
			case TextureSlot::Occlusion: { m_properties.useOcclusionMap = texture.isValid() ? 1.0f : 0.0f; break; }
		}

		m_dirty = true;
	}

	AssetHandle<Texture2D> Material::getTexture(TextureSlot slot) const {
		auto it = m_textures.find(slot);
		if (it != m_textures.end()) {
			return it->second;
		}
		return AssetHandle<Texture2D>();
	}

	Ref<Material> Material::create(const std::string& name, const MaterialProperties& properties) {
		return MakeRef<Material>(name, properties);
	}

	Ref<Material> Material::create(const std::string& name, const AssetHandle<Pipeline>& pipelineHandle, const MaterialProperties& properties) {
		return MakeRef<Material>(name, pipelineHandle, properties);
	}

	uint32_t Material::getTextureBindlessIndex(TextureSlot slot) const {
		auto it = m_textures.find(slot);
		if (it != m_textures.end() && it->second.isValid()) {
			Ref<Texture2D> tex = AssetManager::get<Texture2D>(it->second);
			if (tex) return tex->getBindlessIndex();
		}
		return EngineAssets::getWhiteTexture()->getBindlessIndex();
	}

}
