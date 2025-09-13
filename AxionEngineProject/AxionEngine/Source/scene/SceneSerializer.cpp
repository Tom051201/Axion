#include "axpch.h"
#include "SceneSerializer.h"

#include <fstream>

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/Buffers.h"

namespace YAML {

	template<>
	struct convert<Axion::Vec3> {

		static Node encode(const Axion::Vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, Axion::Vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3) return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}

	};

	template<>
	struct convert<Axion::Vec4> {

		static Node encode(const Axion::Vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, Axion::Vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4) return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}

	};

}

namespace Axion {

	YAML::Emitter& operator<<(YAML::Emitter& out, const Vec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Vec4& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : m_scene(scene) {}

	static void serializeEntity(YAML::Emitter& out, Entity entity) {
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.getComponent<UUIDComponent>().id.toString();

		// -- TagComponent --
		if (entity.hasComponent<TagComponent>()) {
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent
			auto& tag = entity.getComponent<TagComponent>().tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap; // TagComponent
		}

		// -- TransformComponent --
		if (entity.hasComponent<TransformComponent>()) {
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent
			auto& transform = entity.getComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << transform.position;
			out << YAML::Key << "Rotation" << YAML::Value << transform.rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.scale;
			out << YAML::EndMap; // TransformComponent
		}

		// -- MeshComponent --
		if (entity.hasComponent<MeshComponent>()) {
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; // MeshComponent
			auto& mesh = entity.getComponent<MeshComponent>();
			out << YAML::Key << "Path" << YAML::Value << mesh.mesh->getHandle().path;
			out << YAML::EndMap; // MeshComponent
		}

		// -- MaterialComponent --
		if (entity.hasComponent<MaterialComponent>()) {
			out << YAML::Key << "MaterialComponent";
			out << YAML::BeginMap; // MaterialComponent
			auto& mat = entity.getComponent<MaterialComponent>();
			out << YAML::Key << "Name" << YAML::Value << mat.getName();
			out << YAML::EndMap; // MaterialComponent
		}

		// -- ConstantBufferComponent --
		if (entity.hasComponent<ConstantBufferComponent>()) {
			out << YAML::Key << "ConstantBufferComponent";
			out << YAML::BeginMap; // ConstantBufferComponent
			auto& cb = entity.getComponent<MaterialComponent>();
			out << YAML::Key << "Has" << YAML::Value << "TRUE";
			out << YAML::EndMap; // ConstantBufferComponent
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::serializeText(const std::string& filePath) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		// -- Scene name --
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";

		// -- Skybox --
		if (m_scene->m_skybox != nullptr) {
			out << YAML::Key << "Skybox" << YAML::Value << m_scene->m_skybox->getTexturePath();
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

		std::ofstream fout(filePath);
		fout << out.c_str();
	}

	void SceneSerializer::serializeBinary(const std::string& filePath) {
		// TODO: create SceneSerializer::serializeBinary
		AX_CORE_ASSERT(false, "Not implemented yet!");
	}

	bool SceneSerializer::deserializeText(const std::string& filePath) {

		std::ifstream stream(filePath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"]) return false;

		std::string sceneName = data["Scene"].as<std::string>();
		AX_CORE_LOG_TRACE("Deserialized scene {}", sceneName);

		// ----- Skybox -----
		std::string sceneSkybox = data["Skybox"].as<std::string>();
		if (sceneSkybox != "None") {
			AssetHandle<Skybox> skyboxHandle = AssetManager::loadSkybox(sceneSkybox);
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

				AX_CORE_LOG_TRACE("Deserialized entity with ID = {}, name = {}", uuid, name);

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
					AssetHandle<Mesh> handle(meshComponent["Path"].as<std::string>());
					if (!AssetManager::hasMesh(handle)) {
						AssetManager::loadMesh(handle.path);
						AX_CORE_LOG_INFO("LOADED MESH");
					}
					mc.mesh = AssetManager::getMesh(handle);
				}

				// -- MaterialComponent --
				auto materialComponent = entity["MaterialComponent"];
				if (materialComponent) {
					auto& mc = deserializedEntity.addComponent<MaterialComponent>();
					if (materialComponent["Name"].as<std::string>() == "BasicMaterial") {
						// TODO: load basic material which is client side...
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
