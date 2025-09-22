#include "axpch.h"
#include "Material.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/Renderer.h"

namespace Axion {

	Ref<Material> Material::create(const std::string& name, const Vec4& color, const AssetHandle<Shader>& shaderHandle) {
		return std::make_shared<Material>(name, color, shaderHandle);
	}

	Material::Material(const std::string& name, const Vec4& color, const AssetHandle<Shader>& shaderHandle)
	: m_name(name), m_color(color), m_shaderHandle(shaderHandle) {}

	Material::~Material() {
		release();
	}

	void Material::release() {}

	void Material::use() const {
		AssetManager::get<Shader>(m_shaderHandle)->bind();
		Renderer::getSceneDataBuffer()->bind(0);
	}

}
