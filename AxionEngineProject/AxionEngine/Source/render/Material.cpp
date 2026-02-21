#include "axpch.h"
#include "Material.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/Renderer.h"

// TODO: TEMP
#include "AxionEngine/Platform/directx/D12Context.h"

namespace Axion {

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

	void Material::bind() {
		// -- Pipeline --
		Ref<Pipeline> pipeline = AssetManager::get<Pipeline>(m_pipelineHandle);
		if (!pipeline) return;

		pipeline->bind();
		Ref<Shader> shader = pipeline->getSpecification().shader;

		if (!m_slotsCached) {
			m_materialBufferSlot = shader->getBindPoint("MaterialBuffer");
			m_textureTableSlot = shader->getTextureTableBindSlot();

			m_textureIndices[TextureSlot::Albedo] = shader->getBindPoint("t_albedo");
			m_textureIndices[TextureSlot::Normal] = shader->getBindPoint("t_normal");
			m_textureIndices[TextureSlot::Metalness] = shader->getBindPoint("t_metalness");
			m_textureIndices[TextureSlot::Roughness] = shader->getBindPoint("t_roughness");
			m_textureIndices[TextureSlot::Occlusion] = shader->getBindPoint("t_occlusion");
			m_textureIndices[TextureSlot::Emissive] = shader->getBindPoint("t_emissive");

			m_slotsCached = true;
		}

		if (m_materialBufferSlot != -1) {
			if (m_dirty) {
				m_materialBuffer->update(&m_properties, sizeof(MaterialProperties));
				m_dirty = false;
			}
			m_materialBuffer->bind(m_materialBufferSlot);
		}

		// -- Textures --
		std::array<Ref<Texture2D>, 16> textureBatch = {};
		Ref<Texture2D> whiteTex = Renderer::getWhiteFallbackTexture();

		for (uint32_t i = 0; i < 16; i++) {
			textureBatch[i] = whiteTex;
		}

		for (auto const& [slot, texHandle] : m_textures) {
			int texIndex = m_textureIndices[slot];
			if (texIndex != -1 && texIndex < textureBatch.size()) {
				Ref<Texture2D> tex = AssetManager::get<Texture2D>(texHandle);
				textureBatch[texIndex] = tex ? tex : whiteTex;
			}
		}

		Renderer::bindTextures(textureBatch, 16, m_textureTableSlot);
	}

	void Material::unbind() {}

	bool Material::isValid() const {
		return AssetManager::get<Pipeline>(m_pipelineHandle) != nullptr;
	}

	void Material::setTexture(TextureSlot slot, const AssetHandle<Texture2D>& texture) {
		m_textures[slot] = texture;

		switch (slot) {
			case TextureSlot::Normal: {
				m_properties.useNormalMap = texture.isValid() ? 1.0f : 0.0f;
				break;
			}
			case TextureSlot::Metalness: {
				m_properties.useMetalnessMap = texture.isValid() ? 1.0f : 0.0f;
				break;
			}
			case TextureSlot::Roughness: {
				m_properties.useRoughnessMap = texture.isValid() ? 1.0f : 0.0f;
				break;
			}
			case TextureSlot::Occlusion: {
				m_properties.useOcclusionMap = texture.isValid() ? 1.0f : 0.0f;
				break;
			}
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

	Ref<Material> Material::create(const std::string& name, const AssetHandle<Pipeline>& pipelineHandle, const MaterialProperties& properties) {
		return std::make_shared<Material>(name, pipelineHandle, properties);
	}

}
