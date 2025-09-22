#include "axpch.h"
#include "SceneSerializer.h"

#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/YamlHelper.h"
#include "AxionEngine/Source/render/Buffers.h"
#include "AxionEngine/Source/project/ProjectManager.h"

namespace Axion {

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : m_scene(scene) {}

	void SceneSerializer::serializeEntity(YAML::Emitter& out, Entity entity) {
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.getComponent<UUIDComponent>().id.toString();

		// -- TagComponent --
		if (entity.hasComponent<TagComponent>()) {
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			auto& tag = entity.getComponent<TagComponent>().tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap;
		}

		// -- TransformComponent --
		if (entity.hasComponent<TransformComponent>()) {
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
			auto& transform = entity.getComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << transform.position;
			out << YAML::Key << "Rotation" << YAML::Value << transform.rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.scale;
			out << YAML::EndMap;
		}

		// -- MeshComponent --
		if (entity.hasComponent<MeshComponent>()) {
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap;
			auto& mesh = entity.getComponent<MeshComponent>();
			if (mesh.handle.isValid()) {
				std::string relativeMeshPath = AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<Mesh>(mesh.handle));
				out << YAML::Key << "Path" << YAML::Value << relativeMeshPath;
			} else {
				out << YAML::Key << "Path" << YAML::Value << "None";
			}
			out << YAML::EndMap;
		}

		// -- MaterialComponent --
		if (entity.hasComponent<MaterialComponent>()) {
			out << YAML::Key << "MaterialComponent";
			out << YAML::BeginMap;
			auto& mat = entity.getComponent<MaterialComponent>();
			if (mat.handle.isValid()) {
				std::string relativeMatPath = AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<Material>(mat.handle));
				out << YAML::Key << "Path" << YAML::Value << relativeMatPath;
			}
			else {
				out << YAML::Key << "Path" << YAML::Value << "None";
			}
			out << YAML::EndMap;
		}

		// -- ConstantBufferComponent --
		if (entity.hasComponent<ConstantBufferComponent>()) {
			out << YAML::Key << "ConstantBufferComponent";
			out << YAML::BeginMap;
			auto& cb = entity.getComponent<MaterialComponent>();
			out << YAML::Key << "Has" << YAML::Value << "TRUE";
			out << YAML::EndMap;
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::serializeText(const std::string& absoluteFilePath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		// -- Scene name --
		out << YAML::Key << "Scene" << YAML::Value << m_scene->getTitle();

		// -- Skybox --
		if (m_scene->m_skybox != nullptr) {
			std::string relativeSkyboxPath = AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<Skybox>(m_scene->m_skyboxHandle));
			out << YAML::Key << "Skybox" << YAML::Value << relativeSkyboxPath;
		}
		else {
			out << YAML::Key << "Skybox" << YAML::Value << "None";
		}

		// -- Entities --
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		for (auto e : m_scene->m_registry.view<entt::entity>()) {
			Entity entity = { e, m_scene.get() };
			if (!entity) return;
			serializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(absoluteFilePath);
		fout << out.c_str();
	}

	void SceneSerializer::serializeBinary(const std::string& filePath) {
		// TODO: create SceneSerializer::serializeBinary
		AX_CORE_ASSERT(false, "Not implemented yet!");
	}

	bool SceneSerializer::deserializeText(const std::string& absoluteFilePath) {
		std::ifstream stream(absoluteFilePath);
		YAML::Node data = YAML::Load(stream);
		if (!data["Scene"]) return false;

		// ----- Title -----
		std::string sceneName = data["Scene"].as<std::string>();
		m_scene->setTitle(sceneName);



		// ----- Skybox -----
		std::string sceneSkybox = data["Skybox"].as<std::string>();
		if (sceneSkybox != "None") {
			AssetHandle<Skybox> skyboxHandle = AssetManager::load<Skybox>(AssetManager::getAbsolute(sceneSkybox));
			m_scene->setSkybox(skyboxHandle);
		}
		


		// ----- Entities -----
		YAML::Node entities = data["Entities"];
		if (entities) {
			for (auto entity : entities) {
				UUID uuid = UUID::fromString(entity["Entity"].as<std::string>());

				// -- TagComponent --
				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent) {
					name = tagComponent["Tag"].as<std::string>();
				}

				Entity deserializedEntity = m_scene->createEntityWithUUID(name, uuid);

				// -- TransformComponent --
				auto transformComponent = entity["TransformComponent"];
				if (transformComponent) {
					auto& tc = deserializedEntity.getComponent<TransformComponent>();
					tc.position = transformComponent["Translation"].as<Vec3>();
					tc.rotation = transformComponent["Rotation"].as<Vec3>();
					tc.scale = transformComponent["Scale"].as<Vec3>();
				}

				// -- MeshComponent --
				auto meshComponent = entity["MeshComponent"];
				if (meshComponent) {
					auto& mc = deserializedEntity.addComponent<MeshComponent>();
					std::string relPath = meshComponent["Path"].as<std::string>();
					if (relPath != "None") {
						std::string absPath = AssetManager::getAbsolute(relPath);
						AssetHandle<Mesh> handle = AssetManager::load<Mesh>(absPath);
						mc.handle = handle;
					} else {
						mc.handle = AssetHandle<Mesh>();
					}
				}

				// -- MaterialComponent --
				auto materialComponent = entity["MaterialComponent"];
				if (materialComponent) {
					auto& mc = deserializedEntity.addComponent<MaterialComponent>();
					std::string relPath = materialComponent["Path"].as<std::string>();
					if (relPath != "None") {
						std::string absPath = AssetManager::getAbsolute(relPath);
						AssetHandle<Material> handle = AssetManager::load<Material>(absPath);
						mc.handle = handle;
					} else {
						mc.handle = AssetHandle<Material>();
					}
				}

				// -- ConstantBufferComponent --
				auto cbComponent = entity["ConstantBufferComponent"];
				if (cbComponent) {
					auto& cbc = deserializedEntity.addComponent<ConstantBufferComponent>();
					cbc.uploadBuffer = ConstantBuffer::create(sizeof(ObjectBuffer));
				}

			}
		}

		return true;
	}

	bool SceneSerializer::deserializeBinary(const std::string& filePath) {
		// TODO: create SceneSerializer::deserializeBinary
		AX_CORE_ASSERT(false, "Not implemented yet!");
		return false;
	}

}
