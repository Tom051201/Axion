#include "axpch.h"
#include "Material.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/Renderer.h"

// TODO: TEMP
#include "AxionEngine/Platform/directx/D12Context.h"

namespace Axion {

	Material::Material(const std::string& name, const AssetHandle<Shader>& shaderHandle, const MaterialProperties& properties)
		: m_name(name), m_shaderHandle(shaderHandle), m_properties(properties) {

		m_materialBuffer = ConstantBuffer::create(sizeof(MaterialProperties));

		Ref<Texture2D> white = Texture2D::create(32, 32, nullptr);
		m_whiteTexture = AssetManager::insert<Texture2D>(white);
	}

	Material::~Material() {
		release();
	}

	void Material::release() {
		m_materialBuffer->release();
	}

	void Material::bind() {
		// -- Shader --
		Ref<Shader> shader = AssetManager::get<Shader>(m_shaderHandle);
		if (!shader) return;
		shader->bind();

		// -- ConstantBuffer --
		if (m_dirty) {
			m_materialBuffer->update(&m_properties, sizeof(MaterialProperties));
			m_dirty = false;
		}
		m_materialBuffer->bind(3);

		// -- Textures --
		std::array<Ref<Texture2D>, 16> textureBatch = {};
		uint32_t count = static_cast<uint32_t>(TextureSlot::COUNT);
		for (uint32_t i = 0; i < count; i++) {
			TextureSlot slot = (TextureSlot)i;
			if (m_textures.find(slot) != m_textures.end()) {
				textureBatch[i] = AssetManager::get<Texture2D>(m_textures.at(slot));
			}
			else {
				textureBatch[i] = AssetManager::get<Texture2D>(m_whiteTexture);
			}
		}

		// TODO: make this idependent
		if (Renderer::getAPI() == RendererAPI::DirectX12) {
			auto context = static_cast<D12Context*>(GraphicsContext::get()->getNativeContext());
			context->bindSrvTable(2, textureBatch, count);
		}
	}

	void Material::unbind() {}

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

	Ref<Material> Material::create(const std::string& name, const AssetHandle<Shader>& shaderHandle, const MaterialProperties& properties) {
		return std::make_shared<Material>(name, shaderHandle, properties);
	}

}
