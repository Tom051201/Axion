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

		// -- ConstantBuffer --
		if (m_dirty) {
			m_materialBuffer->update(&m_properties, sizeof(MaterialProperties));
			m_dirty = false;
		}
		m_materialBuffer->bind(3);

		// -- Textures --
		std::array<Ref<Texture2D>, 16> textureBatch = {};
		uint32_t count = static_cast<uint32_t>(TextureSlot::COUNT);
		Ref<Texture2D> whiteTex = Renderer::getWhiteFallbackTexture();

		for (uint32_t i = 0; i < count; i++) {
			TextureSlot slot = (TextureSlot)i;
			Ref<Texture2D> tex = nullptr;

			if (m_textures.find(slot) != m_textures.end()) {
				tex = AssetManager::get<Texture2D>(m_textures.at(slot));
			}
			textureBatch[i] = tex ? tex : whiteTex;
		}

		Renderer::bindTextures(textureBatch, count, 2);
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
		}

		m_dirty = true;
	}

	AssetHandle<Texture2D> Material::getTexture(TextureSlot slot) const {

		return AssetHandle<Texture2D>();
	}

	Ref<Material> Material::create(const std::string& name, const AssetHandle<Pipeline>& pipelineHandle, const MaterialProperties& properties) {
		return std::make_shared<Material>(name, pipelineHandle, properties);
	}

}
