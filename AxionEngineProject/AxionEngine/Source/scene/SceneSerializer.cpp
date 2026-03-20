#include "axpch.h"
#include "SceneSerializer.h"

#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/YamlHelper.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/render/Renderer3D.h"
#include "AxionEngine/Source/scripting/NativeScriptRegistry.h"

namespace Axion {

	enum class ComponentID : uint16_t {
		None = 0,
		Tag, Relationship, Transform, Mesh, Sprite, Material, Audio, Camera,
		DirectionalLight, PointLight, SpotLight,
		RigidBody, BoxCollider, SphereCollider, CapsuleCollider, GravitySource,
		Script, NativeScript, ParticleSystem
	};

	static void writeString(std::ofstream& out, const std::string& str) {
		uint32_t len = static_cast<uint32_t>(str.size());
		out.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
		out.write(str.data(), len);
	}

	static std::string readString(std::istream& in) {
		uint32_t len;
		in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
		std::string str(len, '\0');
		in.read(&str[0], len);
		return str;
	}

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

		// -- RelationshipComponent --
		if (entity.hasComponent<RelationshipComponent>()) {
			out << YAML::Key << "RelationshipComponent";
			out << YAML::BeginMap;
			auto& rel = entity.getComponent<RelationshipComponent>();
			if (rel.parent != entt::null) {
				Entity parent = { rel.parent, m_scene.get() };
				out << YAML::Key << "Parent" << YAML::Value << parent.getComponent<UUIDComponent>().id.toString();
			}
			else {
				out << YAML::Key << "Parent" << YAML::Value << "None";
			}
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
				out << YAML::Key << "UUID" << YAML::Value << mesh.handle.uuid;
			} else {
				out << YAML::Key << "UUID" << YAML::Value << "0";
			}
			out << YAML::EndMap;
		}

		// -- SpriteComponent --
		if (entity.hasComponent<SpriteComponent>()) {
			out << YAML::Key << "SpriteComponent";
			out << YAML::BeginMap;
			auto& sprite = entity.getComponent<SpriteComponent>();
			if (sprite.texture.isValid()) {
				out << YAML::Key << "UUID" << YAML::Value << sprite.texture.uuid;
			}
			else {
				out << YAML::Key << "UUID" << YAML::Value << "0";
			}
			out << YAML::Key << "Tint" << YAML::Value << sprite.tint;
			out << YAML::EndMap;
		}

		// -- MaterialComponent --
		if (entity.hasComponent<MaterialComponent>()) {
			out << YAML::Key << "MaterialComponent";
			out << YAML::BeginMap;
			auto& mat = entity.getComponent<MaterialComponent>();
			if (mat.handle.isValid()) {
				out << YAML::Key << "UUID" << YAML::Value << mat.handle.uuid;
			}
			else {
				out << YAML::Key << "UUID" << YAML::Value << "0";
			}
			out << YAML::EndMap;
		}

		// -- AudioComponent --
		if (entity.hasComponent<AudioComponent>()) {
			out << YAML::Key << "AudioComponent";
			out << YAML::BeginMap;
			auto& ac = entity.getComponent<AudioComponent>();
			out << YAML::Key << "IsListener" << YAML::Value << ac.isListener;
			out << YAML::Key << "IsSource" << YAML::Value << ac.isSource;
			if (ac.audio) {
				out << YAML::Key << "AudioSource" << YAML::BeginMap;
				out << YAML::Key << "AudioClip" << YAML::Value << ac.audio->getClipHandle().uuid;
				out << YAML::Key << "Volume" << YAML::Value << ac.audio->getVolume();
				out << YAML::Key << "Pitch" << YAML::Value << ac.audio->getPitch();
				out << YAML::Key << "Pan" << YAML::Value << ac.audio->getPan();
				out << YAML::Key << "IsSpatial" << YAML::Value << ac.audio->isSpatial();
				out << YAML::Key << "Position" << YAML::Value << ac.audio->getPosition();
				out << YAML::Key << "Velocity" << YAML::Value << ac.audio->getVelocity();
				out << YAML::Key << "MinDistance" << YAML::Value << ac.audio->getMinDistance();
				out << YAML::Key << "MaxDistance" << YAML::Value << ac.audio->getMaxDistance();
				out << YAML::Key << "DopplerFactor" << YAML::Value << ac.audio->getDopplerFactor();
				out << YAML::EndMap;
			}
			else {
				out << YAML::Key << "AudioSource" << YAML::Value << "0";
			}
			out << YAML::EndMap;
		}

		// -- Camera Component --
		if (entity.hasComponent<CameraComponent>()) {
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			auto& cc = entity.getComponent<CameraComponent>();
			out << YAML::Key << "Primary" << YAML::Value << cc.isPrimary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.fixedAspectRatio;

			auto& camera = cc.camera;
			out << YAML::Key << "ProjectionType" << YAML::Value << EnumUtils::toString(camera.getProjectionType());
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.getPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.getPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.getPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.getOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.getOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.getOrthographicFarClip();

			out << YAML::EndMap;
		}

		// -- DirectionalLightComponent --
		if (entity.hasComponent<DirectionalLightComponent>()) {
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;
			auto& dlc = entity.getComponent<DirectionalLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << dlc.color;
			out << YAML::EndMap;
		}

		// -- PointLightComponent --
		if (entity.hasComponent<PointLightComponent>()) {
			out << YAML::Key << "PointLightComponent";
			out << YAML::BeginMap;
			auto& plc = entity.getComponent<PointLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << plc.color;
			out << YAML::Key << "Intensity" << YAML::Value << plc.intensity;
			out << YAML::Key << "Radius" << YAML::Value << plc.radius;
			out << YAML::Key << "Falloff" << YAML::Value << plc.falloff;
			out << YAML::EndMap;
		}

		// -- SpotLightComponent --
		if (entity.hasComponent<SpotLightComponent>()) {
			out << YAML::Key << "SpotLightComponent";
			out << YAML::BeginMap;
			auto& slc = entity.getComponent<SpotLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << slc.color;
			out << YAML::Key << "Intensity" << YAML::Value << slc.intensity;
			out << YAML::Key << "Range" << YAML::Value << slc.range;
			out << YAML::Key << "InnerConeAngle" << YAML::Value << slc.innerConeAngle;
			out << YAML::Key << "OuterConeAngle" << YAML::Value << slc.outerConeAngle;
			out << YAML::EndMap;
		}

		// -- RigidBodyComponent --
		if (entity.hasComponent<RigidBodyComponent>()) {
			out << YAML::Key << "RigidBodyComponent";
			out << YAML::BeginMap;
			auto& rbc = entity.getComponent<RigidBodyComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << EnumUtils::toString(rbc.type);
			out << YAML::Key << "Mass" << YAML::Value << rbc.mass;
			out << YAML::Key << "IsKinematic" << YAML::Value << rbc.isKinematic;
			out << YAML::Key << "LinearDamping" << YAML::Value << rbc.linearDamping;
			out << YAML::Key << "AngularDamping" << YAML::Value << rbc.angularDamping;
			out << YAML::Key << "FixedRotationX" << YAML::Value << rbc.fixedRotationX;
			out << YAML::Key << "FixedRotationY" << YAML::Value << rbc.fixedRotationY;
			out << YAML::Key << "FixedRotationZ" << YAML::Value << rbc.fixedRotationZ;
			out << YAML::Key << "EnableCCD" << YAML::Value << rbc.enableCCD;
			out << YAML::EndMap;
		}

		// -- BoxColliderComponent --
		if (entity.hasComponent<BoxColliderComponent>()) {
			out << YAML::Key << "BoxColliderComponent";
			out << YAML::BeginMap;
			auto& bcc = entity.getComponent<BoxColliderComponent>();
			out << YAML::Key << "HalfExtents" << YAML::Value << bcc.halfExtents;
			out << YAML::Key << "Offset" << YAML::Value << bcc.offset;
			out << YAML::Key << "IsTrigger" << YAML::Value << bcc.isTrigger;
			if (bcc.material.isValid()) {
				out << YAML::Key << "Material" << YAML::Value << bcc.material.uuid;
			}
			else {
				out << YAML::Key << "Material" << YAML::Value << "0";
			}
			out << YAML::EndMap;
		}

		// -- SphereColliderComponent --
		if (entity.hasComponent<SphereColliderComponent>()) {
			out << YAML::Key << "SphereColliderComponent";
			out << YAML::BeginMap;
			auto& scc = entity.getComponent<SphereColliderComponent>();
			out << YAML::Key << "Radius" << YAML::Value << scc.radius;
			out << YAML::Key << "Offset" << YAML::Value << scc.offset;
			out << YAML::Key << "IsTrigger" << YAML::Value << scc.isTrigger;
			if (scc.material.isValid()) {
				out << YAML::Key << "Material" << YAML::Value << scc.material.uuid;
			}
			else {
				out << YAML::Key << "Material" << YAML::Value << "0";
			}
			out << YAML::EndMap;
		}

		// -- CapsuleColliderComponent --
		if (entity.hasComponent<CapsuleColliderComponent>()) {
			out << YAML::Key << "CapsuleColliderComponent";
			out << YAML::BeginMap;
			auto& ccc = entity.getComponent<CapsuleColliderComponent>();
			out << YAML::Key << "Radius" << YAML::Value << ccc.radius;
			out << YAML::Key << "HalfHeight" << YAML::Value << ccc.halfHeight;
			out << YAML::Key << "Offset" << YAML::Value << ccc.offset;
			out << YAML::Key << "IsTrigger" << YAML::Value << ccc.isTrigger;
			if (ccc.material.isValid()) {
				out << YAML::Key << "Material" << YAML::Value << ccc.material.uuid;
			}
			else {
				out << YAML::Key << "Material" << YAML::Value << "0";
			}
			out << YAML::EndMap;
		}

		// -- GravitySourceComponent --
		if (entity.hasComponent<GravitySourceComponent>()) {
			out << YAML::Key << "GravitySourceComponent";
			out << YAML::BeginMap;
			auto& gsc = entity.getComponent<GravitySourceComponent>();
			out << YAML::Key << "Type" << YAML::Value << EnumUtils::toString(gsc.type);
			out << YAML::Key << "Strength" << YAML::Value << gsc.strength;
			out << YAML::Key << "Radius" << YAML::Value << gsc.radius;
			out << YAML::Key << "AffectKinematic" << YAML::Value << gsc.affectKinematic;
			out << YAML::EndMap;
		}

		// -- ScriptComponent --
		if (entity.hasComponent<ScriptComponent>()) {
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap;
			auto& sc = entity.getComponent<ScriptComponent>();
			out << YAML::Key << "ClassName" << YAML::Value << sc.className;
			out << YAML::EndMap;
		}

		// -- NativeScriptComponent --
		if (entity.hasComponent<NativeScriptComponent>()) {
			out << YAML::Key << "NativeScriptComponent";
			out << YAML::BeginMap;
			auto& nsc = entity.getComponent<NativeScriptComponent>();
			out << YAML::Key << "ScriptName" << YAML::Value << nsc.scriptName;
			out << YAML::EndMap;
		}

		// -- ParticleSystemComponent --
		if (entity.hasComponent<ParticleSystemComponent>()) {
			out << YAML::Key << "ParticleSystemComponent";
			out << YAML::BeginMap;
			auto& psc = entity.getComponent<ParticleSystemComponent>();
			out << YAML::Key << "VelocityVariation" << YAML::Value << psc.velocityVariation;
			out << YAML::Key << "ColorBegin" << YAML::Value << psc.colorBegin;
			out << YAML::Key << "ColorEnd" << YAML::Value << psc.colorEnd;
			out << YAML::Key << "SizeBegin" << YAML::Value << psc.sizeBegin;
			out << YAML::Key << "SizeEnd" << YAML::Value << psc.sizeEnd;
			out << YAML::Key << "LifeTime" << YAML::Value << psc.lifeTime;
			if (psc.texture.isValid()) {
				out << YAML::Key << "Texture" << YAML::Value << psc.texture.uuid;
			}
			else {
				out << YAML::Key << "Texture" << YAML::Value << "0";
			}
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
		if (m_scene->m_skyboxHandle.isValid()) {
			out << YAML::Key << "Skybox" << YAML::Value << m_scene->m_skyboxHandle.uuid;
		}
		else {
			out << YAML::Key << "Skybox" << YAML::Value << "0";
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
		std::ofstream out(filePath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to open file for Scene binary serialization: {}", filePath);
			return;
		}

		// -- Write Header and Scene Title --
		char magic[4] = { 'A', 'X', 'S', 'N' };
		out.write(magic, 4);
		uint32_t version = ASSET_VERSION_SCENE;
		out.write(reinterpret_cast<const char*>(&version), sizeof(uint32_t));
		writeString(out, m_scene->getTitle());

		// -- Write Skybox --
		UUID skyboxUUID = m_scene->m_skyboxHandle.isValid() ? m_scene->m_skyboxHandle.uuid : UUID(0, 0);
		out.write(reinterpret_cast<const char*>(&skyboxUUID), sizeof(UUID));

		// -- Write Entity Count --
		uint32_t entityCount = 0;
		for (auto e : m_scene->getRegistry().view<TagComponent>()) {
			entityCount++;
		}
		out.write(reinterpret_cast<const char*>(&entityCount), sizeof(uint32_t));

		// -- Serializes Entities --
		for (auto e : m_scene->m_registry.view<TagComponent>()) {
			Entity entity = { e, m_scene.get() };
			if (!entity) continue;

			serializeEntityBinary(out, entity);

		}

	}

	bool SceneSerializer::deserializeText(const std::string& absoluteFilePath) {
		std::ifstream stream(absoluteFilePath);
		YAML::Node data = YAML::Load(stream);
		if (!data["Scene"]) return false;

		auto registry = ProjectManager::getProject()->getAssetRegistry();

		// ----- Title -----
		std::string sceneName = data["Scene"].as<std::string>();
		m_scene->setTitle(sceneName);



		// ----- Skybox -----
		UUID skyboxUUID = data["Skybox"].as<UUID>();
		if (skyboxUUID.isValid()) {
			if (registry->contains(skyboxUUID)) {
				AssetHandle<Skybox> skyboxHandle = AssetManager::load<Skybox>(skyboxUUID);
				m_scene->setSkybox(skyboxHandle);
			}
			else {
				AX_CORE_LOG_WARN("Skybox UUID not found in AssetRegistry!");
			}
		}


		// ----- Entities -----
		YAML::Node entities = data["Entities"];
		if (entities) {

			std::unordered_map<std::string, Entity> uuidToEntityMap;
			std::vector<std::pair<Entity, std::string>> relationshipsToBuild;

			for (auto entityNode : entities) {
				Entity deserializedEntity = deserializeEntityNode(m_scene.get(), entityNode, false);

				std::string uuidStr = deserializedEntity.getComponent<UUIDComponent>().id.toString();
				uuidToEntityMap[uuidStr] = deserializedEntity;

				// -- RelationshipComponent --
				auto relationshipComponent = entityNode["RelationshipComponent"];
				if (relationshipComponent) {
					std::string parentUUID = relationshipComponent["Parent"].as<std::string>();
					if (parentUUID != "None") {
						relationshipsToBuild.push_back({ deserializedEntity, parentUUID });
					}
				}

				// -- Reconstruct hierarchy --
				for (auto& pair : relationshipsToBuild) {
					Entity child = pair.first;
					std::string parentUUID = pair.second;
					if (uuidToEntityMap.find(parentUUID) != uuidToEntityMap.end()) {
						Entity parent = uuidToEntityMap[parentUUID];
						child.setParent(parent);
					}
				}
			}
		}

		return true;
	}

	bool SceneSerializer::deserializeBinary(const std::string& filePath) {
		std::ifstream in(filePath, std::ios::in | std::ios::binary);
		if (!in) {
			AX_CORE_LOG_ERROR("Failed to open file for Scene binary deserialization: {}", filePath);
			return false;
		}

		// -- Verify Header --
		char magic[4];
		in.read(magic, 4);
		if (memcmp(magic, "AXSN", 4) != 0) {
			AX_CORE_LOG_ERROR("Magic of Scene binary is wrong!");
			return false;
		}

		uint32_t version;
		in.read(reinterpret_cast<char*>(&version), sizeof(uint32_t));
		if (version != ASSET_VERSION_SCENE) {
			AX_CORE_LOG_ERROR("Scene version of Scene Binary is not supported");
			return false;
		}

		// -- Read Title and Skybox --
		m_scene->setTitle(readString(in));
		UUID skyboxUUID;
		in.read(reinterpret_cast<char*>(&skyboxUUID), sizeof(UUID));
		if (skyboxUUID.isValid() && ProjectManager::getProject()->getAssetRegistry()->contains(skyboxUUID)) {
			m_scene->setSkybox(AssetManager::load<Skybox>(skyboxUUID));
		}

		// -- Read Entities --
		uint32_t entityCount;
		in.read(reinterpret_cast<char*>(&entityCount), sizeof(uint32_t));

		std::unordered_map<UUID, Entity> uuidToEntityMap;
		std::vector<std::pair<Entity, UUID>> relationshipsToBuild;

		for (uint32_t i = 0; i < entityCount; i++) {

			Entity entity = deserializeEntityBinary(m_scene.get(), in, false, relationshipsToBuild);
			UUID entityUUID = entity.getComponent<UUIDComponent>().id;
			uuidToEntityMap[entityUUID] = entity;

		}

		// -- Reconstruct Hierarchy --
		for (auto& pair : relationshipsToBuild) {
			Entity child = pair.first;
			UUID parentUUID = pair.second;
			if (uuidToEntityMap.find(parentUUID) != uuidToEntityMap.end()) {
				Entity parent = uuidToEntityMap[parentUUID];
				child.setParent(parent);
			}
		}

		return true;
	}

	Entity SceneSerializer::deserializeEntityNode(Scene* scene, YAML::Node& entityNode, bool generateNewUUID) {
		UUID originalUUID = UUID::fromString(entityNode["Entity"].as<std::string>());
		UUID finalUUID = generateNewUUID ? UUID::generate() : originalUUID;

		auto registry = ProjectManager::getProject()->getAssetRegistry();

		// -- TagComponent --
		std::string name;
		auto tagComponent = entityNode["TagComponent"];
		if (tagComponent) {
			name = tagComponent["Tag"].as<std::string>();
		}

		Entity deserializedEntity = scene->createEntityWithUUID(name, finalUUID);

		// -- TransformComponent --
		auto transformComponent = entityNode["TransformComponent"];
		if (transformComponent) {
			auto& tc = deserializedEntity.getComponent<TransformComponent>();
			tc.position = transformComponent["Translation"].as<Vec3>();

			auto rotationNode = transformComponent["Rotation"];
			if (rotationNode.IsSequence() && rotationNode.size() == 3) {
				Vec3 eulerRotation = rotationNode.as<Vec3>();
				tc.setEulerAngles(eulerRotation);
			}
			else {
				tc.rotation = rotationNode.as<Quat>();
			}

			tc.scale = transformComponent["Scale"].as<Vec3>();
		}

		// -- MeshComponent --
		auto meshComponent = entityNode["MeshComponent"];
		if (meshComponent) {
			auto& mc = deserializedEntity.addComponent<MeshComponent>();
			UUID meshUUID = meshComponent["UUID"].as<UUID>();
			if (meshUUID.isValid()) {
				if (registry->contains(meshUUID)) {
					AssetHandle<Mesh> handle = AssetManager::load<Mesh>(meshUUID);
					mc.handle = handle;
				}
				else {
					mc.handle = AssetHandle<Mesh>();
				}
			}
			else {
				mc.handle = AssetHandle<Mesh>();
			}
		}

		// -- SpriteComponent --
		auto spriteComponent = entityNode["SpriteComponent"];
		if (spriteComponent) {
			auto& sc = deserializedEntity.addComponent<SpriteComponent>();
			UUID spriteUUID = spriteComponent["UUID"].as<UUID>();
			if (spriteUUID.isValid()) {
				if (registry->contains(spriteUUID)) {
					AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(spriteUUID);
					sc.texture = handle;
				}
				else {
					sc.texture = AssetHandle<Texture2D>();
				}
			}
			else {
				sc.texture = AssetHandle<Texture2D>();
			}
			sc.tint = spriteComponent["Tint"].as<Vec4>();
		}

		// -- MaterialComponent --
		auto materialComponent = entityNode["MaterialComponent"];
		if (materialComponent) {
			auto& mc = deserializedEntity.addComponent<MaterialComponent>();
			UUID materialUUID = materialComponent["UUID"].as<UUID>();
			if (materialUUID.isValid()) {
				if (registry->contains(materialUUID)) {
					AssetHandle<Material> handle = AssetManager::load<Material>(materialUUID);
					mc.handle = handle;
				}
				else {
					mc.handle = AssetHandle<Material>();
				}
			}
			else {
				mc.handle = AssetHandle<Material>();
			}
		}

		// -- AudioComponent --
		auto audioComponent = entityNode["AudioComponent"];
		if (audioComponent) {
			auto& ac = deserializedEntity.addComponent<AudioComponent>();
			ac.isListener = audioComponent["IsListener"].as<bool>();
			ac.isSource = audioComponent["IsSource"].as<bool>();

			auto as = audioComponent["AudioSource"];
			if (as && as.IsScalar() && as.as<std::string>() == "None") {
				ac.audio = nullptr;
			}
			else {
				UUID clipUUID = as["AudioClip"].as<UUID>();
				if (clipUUID.isValid()) {
					if (registry->contains(clipUUID)) {
						AssetHandle<AudioClip> handle = AssetManager::load<AudioClip>(clipUUID);
						ac.audio = std::make_shared<AudioSource>(handle);
						ac.audio->setVolume(as["Volume"].as<float>());
						ac.audio->setPitch(as["Pitch"].as<float>());
						ac.audio->setPan(as["Pan"].as<float>());
						if (as["IsSpatial"].as<bool>()) {
							ac.audio->enableSpatial();
						}
						else {
							ac.audio->disableSpatial();
						}
						ac.audio->setPosition(as["Position"].as<Vec3>());
						ac.audio->setVelocity(as["Velocity"].as<Vec3>());
						ac.audio->setMinDistance(as["MinDistance"].as<float>());
						ac.audio->setMaxDistance(as["MaxDistance"].as<float>());
						ac.audio->setDopplerFactor(as["DopplerFactor"].as<float>());
					}
					else {
						ac.audio = nullptr;
					}
				}
				else {
					ac.audio = nullptr;
				}
			}
		}

		// -- CameraComponent --
		auto cameraComponent = entityNode["CameraComponent"];
		if (cameraComponent) {
			auto& cc = deserializedEntity.addComponent<CameraComponent>();
			cc.isPrimary = cameraComponent["Primary"].as<bool>();
			cc.fixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();

			if (cameraComponent["ProjectionType"]) {
				cc.camera.setProjectionType(EnumUtils::cameraProjectionTypeFromString(cameraComponent["ProjectionType"].as<std::string>()));

				cc.camera.setPerspectiveVerticalFOV(cameraComponent["PerspectiveFOV"].as<float>());
				cc.camera.setPerspectiveNearClip(cameraComponent["PerspectiveNear"].as<float>());
				cc.camera.setPerspectiveFarClip(cameraComponent["PerspectiveFar"].as<float>());

				cc.camera.setOrthographicSize(cameraComponent["OrthographicSize"].as<float>());
				cc.camera.setOrthographicNearClip(cameraComponent["OrthographicNear"].as<float>());
				cc.camera.setOrthographicFarClip(cameraComponent["OrthographicFar"].as<float>());
			}
		}

		// -- DirectionalLightComponent --
		auto directionalLightComponent = entityNode["DirectionalLightComponent"];
		if (directionalLightComponent) {
			auto& dlc = deserializedEntity.addComponent<DirectionalLightComponent>();
			dlc.color = directionalLightComponent["Color"].as<Vec4>();
		}

		// -- PointLightComponent --
		auto pointLightComponent = entityNode["PointLightComponent"];
		if (pointLightComponent) {
			auto& plc = deserializedEntity.addComponent<PointLightComponent>();
			plc.color = pointLightComponent["Color"].as<Vec4>();
			plc.intensity = pointLightComponent["Intensity"].as<float>();
			plc.radius = pointLightComponent["Radius"].as<float>();
			plc.falloff = pointLightComponent["Falloff"].as<float>();
		}

		// -- SpotLightComponent --
		auto spotLightComponent = entityNode["SpotLightComponent"];
		if (spotLightComponent) {
			auto& slc = deserializedEntity.addComponent<SpotLightComponent>();
			slc.color = spotLightComponent["Color"].as<Vec4>();
			slc.intensity = spotLightComponent["Intensity"].as<float>();
			slc.range = spotLightComponent["Range"].as<float>();
			slc.innerConeAngle = spotLightComponent["InnerConeAngle"].as<float>();
			slc.outerConeAngle = spotLightComponent["OuterConeAngle"].as<float>();
		}

		// -- RigidBodyComponent --
		auto rigidBodyComponent = entityNode["RigidBodyComponent"];
		if (rigidBodyComponent) {
			auto& rbc = deserializedEntity.addComponent<RigidBodyComponent>();
			rbc.type = EnumUtils::rigidBodyTypeFromString(rigidBodyComponent["BodyType"].as<std::string>());
			rbc.mass = rigidBodyComponent["Mass"].as<float>();
			rbc.isKinematic = rigidBodyComponent["IsKinematic"].as<bool>();
			rbc.linearDamping = rigidBodyComponent["LinearDamping"].as<float>();
			rbc.angularDamping = rigidBodyComponent["AngularDamping"].as<float>();
			rbc.fixedRotationX = rigidBodyComponent["FixedRotationX"].as<bool>();
			rbc.fixedRotationY = rigidBodyComponent["FixedRotationY"].as<bool>();
			rbc.fixedRotationZ = rigidBodyComponent["FixedRotationZ"].as<bool>();
			rbc.enableCCD = rigidBodyComponent["EnableCCD"].as<bool>();
		}

		// -- BoxColliderComponent --
		auto boxColliderComponent = entityNode["BoxColliderComponent"];
		if (boxColliderComponent) {
			auto& bcc = deserializedEntity.addComponent<BoxColliderComponent>();
			bcc.halfExtents = boxColliderComponent["HalfExtents"].as<Vec3>();
			bcc.offset = boxColliderComponent["Offset"].as<Vec3>();
			bcc.isTrigger = boxColliderComponent["IsTrigger"].as<bool>();
			UUID materialUUID = boxColliderComponent["Material"].as<UUID>();
			if (materialUUID.isValid()) {
				if (registry->contains(materialUUID)) {
					AssetHandle<PhysicsMaterial> handle = AssetManager::load<PhysicsMaterial>(materialUUID);
					bcc.material = handle;
				}
				else {
					bcc.material = AssetHandle<PhysicsMaterial>();
				}
			}
			else {
				bcc.material = AssetHandle<PhysicsMaterial>();
			}
		}

		// -- SphereColliderComponent --
		auto sphereColliderComponent = entityNode["SphereColliderComponent"];
		if (sphereColliderComponent) {
			auto& scc = deserializedEntity.addComponent<SphereColliderComponent>();
			scc.radius = sphereColliderComponent["Radius"].as<float>();
			scc.offset = sphereColliderComponent["Offset"].as<Vec3>();
			scc.isTrigger = sphereColliderComponent["IsTrigger"].as<bool>();
			UUID materialUUID = sphereColliderComponent["Material"].as<UUID>();
			if (materialUUID.isValid()) {
				if (registry->contains(materialUUID)) {
					AssetHandle<PhysicsMaterial> handle = AssetManager::load<PhysicsMaterial>(materialUUID);
					scc.material = handle;
				}
				else {
					scc.material = AssetHandle<PhysicsMaterial>();
				}
			}
			else {
				scc.material = AssetHandle<PhysicsMaterial>();
			}
		}

		// -- CapsuleColliderComponent --
		auto capsuleColliderComponent = entityNode["CapsuleColliderComponent"];
		if (capsuleColliderComponent) {
			auto& ccc = deserializedEntity.addComponent<CapsuleColliderComponent>();
			ccc.radius = capsuleColliderComponent["Radius"].as<float>();
			ccc.halfHeight = capsuleColliderComponent["HalfHeight"].as<float>();
			ccc.offset = capsuleColliderComponent["Offset"].as<Vec3>();
			ccc.isTrigger = capsuleColliderComponent["IsTrigger"].as<bool>();
			UUID materialUUID = capsuleColliderComponent["Material"].as<UUID>();
			if (materialUUID.isValid()) {
				if (registry->contains(materialUUID)) {
					AssetHandle<PhysicsMaterial> handle = AssetManager::load<PhysicsMaterial>(materialUUID);
					ccc.material = handle;
				}
				else {
					ccc.material = AssetHandle<PhysicsMaterial>();
				}
			}
			else {
				ccc.material = AssetHandle<PhysicsMaterial>();
			}
		}

		// -- GravitySourceComponent --
		auto gravitySourceComponent = entityNode["GravitySourceComponent"];
		if (gravitySourceComponent) {
			auto& gsc = deserializedEntity.addComponent<GravitySourceComponent>();
			gsc.type = EnumUtils::gravitySourceTypeFromString(gravitySourceComponent["Type"].as<std::string>());
			gsc.strength = gravitySourceComponent["Strength"].as<float>();
			gsc.radius = gravitySourceComponent["Radius"].as<float>();
			gsc.affectKinematic = gravitySourceComponent["AffectKinematic"].as<bool>();
		}

		// -- ScriptComponent --
		auto scriptComponent = entityNode["ScriptComponent"];
		if (scriptComponent) {
			auto& sc = deserializedEntity.addComponent<ScriptComponent>();
			sc.className = scriptComponent["ClassName"].as<std::string>();
		}

		// -- NativeScriptComponent --
		auto nativeScriptComponent = entityNode["NativeScriptComponent"];
		if (nativeScriptComponent) {
			auto& nsc = deserializedEntity.addComponent<NativeScriptComponent>();
			std::string scriptName = nativeScriptComponent["ScriptName"].as<std::string>();
			if (scriptName != "None") {
				NativeScriptRegistry::bind(deserializedEntity, scriptName);
			}
		}

		// -- ParticleSystemComponent --
		auto particleSystemComponent = entityNode["ParticleSystemComponent"];
		if (particleSystemComponent) {
			auto& psc = deserializedEntity.addComponent<ParticleSystemComponent>();

			psc.velocityVariation = particleSystemComponent["VelocityVariation"].as<Vec3>();
			psc.colorBegin = particleSystemComponent["ColorBegin"].as<Vec4>();
			psc.colorEnd = particleSystemComponent["ColorEnd"].as<Vec4>();
			psc.sizeBegin = particleSystemComponent["SizeBegin"].as<float>();
			psc.sizeEnd = particleSystemComponent["SizeEnd"].as<float>();
			psc.lifeTime = particleSystemComponent["LifeTime"].as<float>();

			UUID textureUUID = particleSystemComponent["Texture"].as<UUID>();
			if (textureUUID.isValid()) {
				if (registry->contains(textureUUID)) {
					AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(textureUUID);
					psc.texture = handle;
				}
				else {
					psc.texture = AssetHandle<Texture2D>();
				}
			}
			else {
				psc.texture = AssetHandle<Texture2D>();
			}
		}

		return deserializedEntity;
	}

	void SceneSerializer::serializeEntityBinary(std::ofstream& out, Entity entity) {
		// -- Write Entity UUID --
		UUID entityUUID = entity.getComponent<UUIDComponent>().id;
		out.write(reinterpret_cast<const char*>(&entityUUID), sizeof(UUID));

		// -- Write Transform Component --
		if (entity.hasComponent<TransformComponent>()) {
			ComponentID id = ComponentID::Transform;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<TransformComponent>();
			out.write(reinterpret_cast<const char*>(&component.position), sizeof(Vec3));
			out.write(reinterpret_cast<const char*>(&component.rotation), sizeof(Quat));
			out.write(reinterpret_cast<const char*>(&component.scale), sizeof(Vec3));
		}

		// -- Write Tag Component --
		if (entity.hasComponent<TagComponent>()) {
			ComponentID id = ComponentID::Tag;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			writeString(out, entity.getComponent<TagComponent>().tag);
		}

		// -- Write Relationship Component --
		if (entity.hasComponent<RelationshipComponent>()) {
			ComponentID id = ComponentID::Relationship;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<RelationshipComponent>();
			UUID parentUUID = (component.parent != entt::null) ? Entity{ component.parent, m_scene.get() }.getComponent<UUIDComponent>().id : UUID(0, 0);
			out.write(reinterpret_cast<const char*>(&parentUUID), sizeof(UUID));
		}

		// -- Write Mesh Component --
		if (entity.hasComponent<MeshComponent>()) {
			ComponentID id = ComponentID::Mesh;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			UUID meshUUID = entity.getComponent<MeshComponent>().handle.isValid() ? entity.getComponent<MeshComponent>().handle.uuid : UUID(0, 0);
			out.write(reinterpret_cast<const char*>(&meshUUID), sizeof(UUID));
		}

		// -- Write Sprite Component --
		if (entity.hasComponent<SpriteComponent>()) {
			ComponentID id = ComponentID::Sprite;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<SpriteComponent>();
			UUID textureUUID = component.texture.isValid() ? component.texture.uuid : UUID(0, 0);
			out.write(reinterpret_cast<const char*>(&textureUUID), sizeof(UUID));
			Vec4 tint = component.tint;
			out.write(reinterpret_cast<const char*>(&tint), sizeof(Vec4));
		}

		// -- Write Material Component --
		if (entity.hasComponent<MaterialComponent>()) {
			ComponentID id = ComponentID::Material;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			UUID materialUUID = entity.getComponent<MaterialComponent>().handle.isValid() ? entity.getComponent<MaterialComponent>().handle.uuid : UUID(0, 0);
			out.write(reinterpret_cast<const char*>(&materialUUID), sizeof(UUID));
		}

		// -- Write Audio Component --
		if (entity.hasComponent<AudioComponent>()) {
			ComponentID id = ComponentID::Audio;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<AudioComponent>();
			bool hasAudio = component.audio != nullptr;
			uint8_t flags =
				(component.isListener ? 1 : 0) |
				((component.isSource ? 1 : 0) << 1) |
				((hasAudio ? 1 : 0) << 1);
			bool isListener = component.isListener;
			bool isSource = component.isSource;
			out.write(reinterpret_cast<const char*>(&flags), sizeof(uint8_t));
			if (hasAudio) {
				UUID clipUUID = component.audio->m_clipHandle.isValid() ? component.audio->m_clipHandle.uuid : UUID(0, 0);
				out.write(reinterpret_cast<const char*>(&clipUUID), sizeof(UUID));
				out.write(reinterpret_cast<const char*>(&component.audio->m_volume), sizeof(float));
				out.write(reinterpret_cast<const char*>(&component.audio->m_pitch), sizeof(float));
				out.write(reinterpret_cast<const char*>(&component.audio->m_pan), sizeof(float));
				bool isSpacial = component.audio->isSpatial();
				out.write(reinterpret_cast<const char*>(&isSpacial), sizeof(bool));
				out.write(reinterpret_cast<const char*>(&component.audio->m_position), sizeof(Vec3));
				out.write(reinterpret_cast<const char*>(&component.audio->m_velocity), sizeof(Vec3));
				out.write(reinterpret_cast<const char*>(&component.audio->m_minDistance), sizeof(float));
				out.write(reinterpret_cast<const char*>(&component.audio->m_maxDistance), sizeof(float));
				out.write(reinterpret_cast<const char*>(&component.audio->m_dopplerFactor), sizeof(float));
			}
		}

		// -- Write Camera Component --
		if (entity.hasComponent<CameraComponent>()) {
			ComponentID id = ComponentID::Camera;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<CameraComponent>();
			uint8_t flags =
				(component.isPrimary ? 1 : 0) |
				((component.fixedAspectRatio ? 1 : 0) << 1);
			out.write(reinterpret_cast<const char*>(&flags), sizeof(uint8_t));
			uint32_t projectionTypeInt = static_cast<uint32_t>(component.camera.getProjectionType());
			float perspectiveFOV = component.camera.getPerspectiveVerticalFOV();
			float perspectiveNear = component.camera.getPerspectiveNearClip();
			float perspectiveFar = component.camera.getPerspectiveFarClip();
			float orthoSize = component.camera.getOrthographicSize();
			float orthoNear = component.camera.getOrthographicNearClip();
			float orthoFar = component.camera.getOrthographicFarClip();
			out.write(reinterpret_cast<const char*>(&projectionTypeInt), sizeof(uint32_t));
			out.write(reinterpret_cast<const char*>(&perspectiveFOV), sizeof(float));
			out.write(reinterpret_cast<const char*>(&perspectiveNear), sizeof(float));
			out.write(reinterpret_cast<const char*>(&perspectiveFar), sizeof(float));
			out.write(reinterpret_cast<const char*>(&orthoSize), sizeof(float));
			out.write(reinterpret_cast<const char*>(&orthoNear), sizeof(float));
			out.write(reinterpret_cast<const char*>(&orthoFar), sizeof(float));
		}

		// -- Write Directional Light Component --
		if (entity.hasComponent<DirectionalLightComponent>()) {
			ComponentID id = ComponentID::DirectionalLight;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			Vec4 color = entity.getComponent<DirectionalLightComponent>().color;
			out.write(reinterpret_cast<const char*>(&color), sizeof(Vec4));
		}

		// -- Write Point Light Component --
		if (entity.hasComponent<PointLightComponent>()) {
			ComponentID id = ComponentID::PointLight;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<PointLightComponent>();
			out.write(reinterpret_cast<const char*>(&component.color), sizeof(Vec4));
			out.write(reinterpret_cast<const char*>(&component.intensity), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.radius), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.falloff), sizeof(float));
		}

		// -- Write Spot Light Component --
		if (entity.hasComponent<SpotLightComponent>()) {
			ComponentID id = ComponentID::SpotLight;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<SpotLightComponent>();
			out.write(reinterpret_cast<const char*>(&component.color), sizeof(Vec4));
			out.write(reinterpret_cast<const char*>(&component.intensity), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.range), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.innerConeAngle), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.outerConeAngle), sizeof(float));
		}

		// -- Write Rigid Body Component --
		if (entity.hasComponent<RigidBodyComponent>()) {
			ComponentID id = ComponentID::RigidBody;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<RigidBodyComponent>();
			uint32_t type = static_cast<uint32_t>(component.type);
			out.write(reinterpret_cast<const char*>(&type), sizeof(uint32_t));
			out.write(reinterpret_cast<const char*>(&component.mass), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.linearDamping), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.angularDamping), sizeof(float));
			uint8_t flags =
				(component.isKinematic ? 1 : 0) |
				((component.fixedRotationX ? 1 : 0) << 1) |
				((component.fixedRotationY ? 1 : 0) << 2) |
				((component.fixedRotationZ ? 1 : 0) << 3) |
				((component.enableCCD ? 1 : 0) << 4);
			out.write(reinterpret_cast<const char*>(&flags), sizeof(uint8_t));
		}

		// -- Write Box Collider Component --
		if (entity.hasComponent<BoxColliderComponent>()) {
			ComponentID id = ComponentID::BoxCollider;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<BoxColliderComponent>();
			UUID materialUUID = component.material.isValid() ? component.material.uuid : UUID(0, 0);
			out.write(reinterpret_cast<const char*>(&materialUUID), sizeof(UUID));
			out.write(reinterpret_cast<const char*>(&component.halfExtents), sizeof(Vec3));
			out.write(reinterpret_cast<const char*>(&component.offset), sizeof(Vec3));
			uint8_t flags = (component.isTrigger ? 1 : 0);
			out.write(reinterpret_cast<const char*>(&flags), sizeof(uint8_t));
		}

		// -- Write Sphere Collider Component --
		if (entity.hasComponent<SphereColliderComponent>()) {
			ComponentID id = ComponentID::SphereCollider;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<SphereColliderComponent>();
			UUID materialUUID = component.material.isValid() ? component.material.uuid : UUID(0, 0);
			out.write(reinterpret_cast<const char*>(&materialUUID), sizeof(UUID));
			out.write(reinterpret_cast<const char*>(&component.radius), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.offset), sizeof(Vec3));
			uint8_t flags = (component.isTrigger ? 1 : 0);
			out.write(reinterpret_cast<const char*>(&flags), sizeof(uint8_t));
		}

		// -- Write Capsule Collider Component --
		if (entity.hasComponent<CapsuleColliderComponent>()) {
			ComponentID id = ComponentID::CapsuleCollider;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<CapsuleColliderComponent>();
			UUID materialUUID = component.material.isValid() ? component.material.uuid : UUID(0, 0);
			out.write(reinterpret_cast<const char*>(&materialUUID), sizeof(UUID));
			out.write(reinterpret_cast<const char*>(&component.radius), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.halfHeight), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.offset), sizeof(Vec3));
			uint8_t flags = (component.isTrigger ? 1 : 0);
			out.write(reinterpret_cast<const char*>(&flags), sizeof(uint8_t));
		}

		// -- Write Gravity Source Component --
		if (entity.hasComponent<GravitySourceComponent>()) {
			ComponentID id = ComponentID::GravitySource;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<GravitySourceComponent>();
			uint32_t typeInt = static_cast<uint32_t>(component.type);
			out.write(reinterpret_cast<const char*>(&typeInt), sizeof(uint32_t));
			out.write(reinterpret_cast<const char*>(&component.strength), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.radius), sizeof(float));
			uint8_t flags = (component.affectKinematic ? 1 : 0);
			out.write(reinterpret_cast<const char*>(&flags), sizeof(uint8_t));
		}

		// -- Write C# Script Component --
		if (entity.hasComponent<ScriptComponent>()) {
			ComponentID id = ComponentID::Script;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			writeString(out, entity.getComponent<ScriptComponent>().className);
		}

		// -- Write Native Script Component --
		if (entity.hasComponent<NativeScriptComponent>()) {
			ComponentID id = ComponentID::NativeScript;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			writeString(out, entity.getComponent<NativeScriptComponent>().scriptName);
		}

		// -- Write Particle System Component --
		if (entity.hasComponent<ParticleSystem>()) {
			ComponentID id = ComponentID::ParticleSystem;
			out.write(reinterpret_cast<const char*>(&id), sizeof(uint16_t));
			auto& component = entity.getComponent<ParticleSystemComponent>();
			UUID textureUUID = component.texture.isValid() ? component.texture.uuid : UUID(0, 0);
			out.write(reinterpret_cast<const char*>(&textureUUID), sizeof(UUID));
			out.write(reinterpret_cast<const char*>(&component.sizeBegin), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.sizeEnd), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.lifeTime), sizeof(float));
			out.write(reinterpret_cast<const char*>(&component.velocityVariation), sizeof(Vec3));
			out.write(reinterpret_cast<const char*>(&component.colorBegin), sizeof(Vec4));
			out.write(reinterpret_cast<const char*>(&component.colorEnd), sizeof(Vec4));
		}
	}

	Entity SceneSerializer::deserializeEntityBinary(Scene* scene, std::istream& in, bool generateNewUUID, std::vector<std::pair<Entity, UUID>>& relationshipsToBuild) {
		UUID originalyUUID;
		in.read(reinterpret_cast<char*>(&originalyUUID), sizeof(UUID));
		UUID finalUUID = generateNewUUID ? UUID::generate() : originalyUUID;

		Entity entity = scene->createEntityWithUUID("Unnamed", finalUUID);

		while (true) {
			ComponentID id;
			in.read(reinterpret_cast<char*>(&id), sizeof(uint16_t));

			if (id == ComponentID::None) break;

			switch (id) {
			case ComponentID::Transform: {
				// -- Read Transform Component --
				auto& component = entity.getComponent<TransformComponent>();
				in.read(reinterpret_cast<char*>(&component.position), sizeof(Vec3));
				in.read(reinterpret_cast<char*>(&component.rotation), sizeof(Quat));
				in.read(reinterpret_cast<char*>(&component.scale), sizeof(Vec3));
				break;
			}
			case ComponentID::Tag: {
				// -- Read Tag Component --
				entity.getComponent<TagComponent>().tag = readString(in);
				break;
			}
			case ComponentID::Relationship: {
				// -- Read Relationship Component --
				UUID parentUUID;
				in.read(reinterpret_cast<char*>(&parentUUID), sizeof(UUID));
				if (parentUUID.isValid()) {
					relationshipsToBuild.push_back({ entity, parentUUID });
				}
				break;
			}
			case ComponentID::Mesh: {
				// -- Read Mesh Component --
				auto& component = entity.addComponent<MeshComponent>();
				UUID meshUUID;
				in.read(reinterpret_cast<char*>(&meshUUID), sizeof(UUID));
				if (meshUUID.isValid()) component.handle = AssetManager::load<Mesh>(meshUUID);
				break;
			}
			case ComponentID::Sprite: {
				// -- Read Sprite Component --
				auto& component = entity.addComponent<SpriteComponent>();
				UUID textureUUID;
				in.read(reinterpret_cast<char*>(&textureUUID), sizeof(UUID));
				if (textureUUID.isValid()) component.texture = AssetManager::load<Texture2D>(textureUUID);
				Vec4 tint;
				in.read(reinterpret_cast<char*>(&tint), sizeof(Vec4));
				component.tint = tint;
				break;
			}
			case ComponentID::Material: {
				// -- Read Material Component --
				auto& component = entity.addComponent<MaterialComponent>();
				UUID materialUUID;
				in.read(reinterpret_cast<char*>(&materialUUID), sizeof(UUID));
				if (materialUUID.isValid()) component.handle = AssetManager::load<Material>(materialUUID);
				break;
			}
			case ComponentID::Audio: {
				// -- Read Audio Component --
				auto& component = entity.addComponent<AudioComponent>();
				bool hasAudio;
				uint8_t flags;
				in.read(reinterpret_cast<char*>(&flags), sizeof(uint8_t));
				component.isListener = (flags & 1) != 0;
				component.isSource = (flags & 2) != 0;
				hasAudio = (flags & 4) != 0;
				if (hasAudio) {
					UUID clipUUID;
					in.read(reinterpret_cast<char*>(&clipUUID), sizeof(UUID));
					if (clipUUID.isValid()) {
						AssetHandle<AudioClip> clipHandle = AssetManager::load<AudioClip>(clipUUID);
						component.audio = std::make_shared<AudioSource>(clipHandle);
						in.read(reinterpret_cast<char*>(&component.audio->m_volume), sizeof(float));
						in.read(reinterpret_cast<char*>(&component.audio->m_pitch), sizeof(float));
						in.read(reinterpret_cast<char*>(&component.audio->m_pan), sizeof(float));
						bool isSpatial;
						in.read(reinterpret_cast<char*>(&isSpatial), sizeof(bool));
						if (isSpatial) { component.audio->enableSpatial(); }
						else { component.audio->disableSpatial(); }
						in.read(reinterpret_cast<char*>(&component.audio->m_position), sizeof(Vec3));
						in.read(reinterpret_cast<char*>(&component.audio->m_velocity), sizeof(Vec3));
						in.read(reinterpret_cast<char*>(&component.audio->m_minDistance), sizeof(float));
						in.read(reinterpret_cast<char*>(&component.audio->m_maxDistance), sizeof(float));
						in.read(reinterpret_cast<char*>(&component.audio->m_dopplerFactor), sizeof(float));
					}
				}
				break;
			}
			case ComponentID::Camera: {
				// -- Read Camera Component --
				auto& component = entity.addComponent<CameraComponent>();
				uint8_t flags;
				in.read(reinterpret_cast<char*>(&flags), sizeof(uint8_t));
				component.isPrimary = (flags & 1) != 0;
				component.fixedAspectRatio = (flags & 2) != 0;
				uint32_t projTypeInt;
				in.read(reinterpret_cast<char*>(&projTypeInt), sizeof(uint32_t));
				component.camera.setProjectionType(static_cast<Camera::ProjectionType>(projTypeInt));
				float perspFOV;
				float perspNear;
				float perspFar;
				float orthoSize;
				float orthoNear;
				float orthoFar;
				in.read(reinterpret_cast<char*>(&perspFOV), sizeof(float));
				in.read(reinterpret_cast<char*>(&perspNear), sizeof(float));
				in.read(reinterpret_cast<char*>(&perspFar), sizeof(float));
				in.read(reinterpret_cast<char*>(&orthoSize), sizeof(float));
				in.read(reinterpret_cast<char*>(&orthoNear), sizeof(float));
				in.read(reinterpret_cast<char*>(&orthoFar), sizeof(float));
				component.camera.setPerspectiveVerticalFOV(perspFOV);
				component.camera.setPerspectiveNearClip(perspNear);
				component.camera.setPerspectiveFarClip(perspFar);
				component.camera.setOrthographicSize(orthoSize);
				component.camera.setOrthographicNearClip(orthoNear);
				component.camera.setOrthographicFarClip(orthoFar);
				break;
			}
			case ComponentID::DirectionalLight: {
				// -- Read Directional Light Component --
				auto& component = entity.addComponent<DirectionalLightComponent>();
				in.read(reinterpret_cast<char*>(&component.color), sizeof(Vec4));
				break;
			}
			case ComponentID::PointLight: {
				// -- Read Point Light Component --
				auto& component = entity.addComponent<PointLightComponent>();
				in.read(reinterpret_cast<char*>(&component.color), sizeof(Vec4));
				in.read(reinterpret_cast<char*>(&component.intensity), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.radius), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.falloff), sizeof(float));
				break;
			}
			case ComponentID::SpotLight: {
				// -- Read Spot Light Component --
				auto& component = entity.addComponent<SpotLightComponent>();
				in.read(reinterpret_cast<char*>(&component.color), sizeof(Vec4));
				in.read(reinterpret_cast<char*>(&component.intensity), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.range), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.innerConeAngle), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.outerConeAngle), sizeof(float));
				break;
			}
			case ComponentID::RigidBody: {
				// -- Read Rigid Body Component --
				auto& component = entity.addComponent<RigidBodyComponent>();
				uint32_t typeInt;
				in.read(reinterpret_cast<char*>(&typeInt), sizeof(uint32_t));
				component.type = static_cast<RigidBodyComponent::BodyType>(typeInt);
				in.read(reinterpret_cast<char*>(&component.mass), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.linearDamping), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.angularDamping), sizeof(float));
				uint8_t flags;
				in.read(reinterpret_cast<char*>(&flags), sizeof(uint8_t));
				component.isKinematic = (flags & 1) != 0;
				component.fixedRotationX = (flags & 2) != 0;
				component.fixedRotationY = (flags & 4) != 0;
				component.fixedRotationZ = (flags & 8) != 0;
				component.enableCCD = (flags & 16) != 0;
				break;
			}
			case ComponentID::BoxCollider: {
				// -- Read Box Collider Component --
				auto& component = entity.addComponent<BoxColliderComponent>();
				UUID materialUUID;
				in.read(reinterpret_cast<char*>(&materialUUID), sizeof(UUID));
				if (materialUUID.isValid()) component.material = AssetManager::load<PhysicsMaterial>(materialUUID);
				in.read(reinterpret_cast<char*>(&component.halfExtents), sizeof(Vec3));
				in.read(reinterpret_cast<char*>(&component.offset), sizeof(Vec3));
				uint8_t flags;
				in.read(reinterpret_cast<char*>(&flags), sizeof(uint8_t));
				component.isTrigger = (flags & 1) != 0;
				break;
			}
			case ComponentID::SphereCollider: {
				// -- Read Sphere Collider Component --
				auto& component = entity.addComponent<SphereColliderComponent>();
				UUID materialUUID;
				in.read(reinterpret_cast<char*>(&materialUUID), sizeof(UUID));
				if (materialUUID.isValid()) component.material = AssetManager::load<PhysicsMaterial>(materialUUID);
				in.read(reinterpret_cast<char*>(&component.radius), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.offset), sizeof(Vec3));
				uint8_t flags;
				in.read(reinterpret_cast<char*>(&flags), sizeof(uint8_t));
				component.isTrigger = (flags & 1) != 0;
				break;
			}
			case ComponentID::CapsuleCollider: {
				// -- Read Capsule Collider Component --
				auto& component = entity.addComponent<CapsuleColliderComponent>();
				UUID materialUUID;
				in.read(reinterpret_cast<char*>(&materialUUID), sizeof(UUID));
				if (materialUUID.isValid()) component.material = AssetManager::load<PhysicsMaterial>(materialUUID);
				in.read(reinterpret_cast<char*>(&component.radius), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.halfHeight), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.offset), sizeof(Vec3));
				uint8_t flags;
				in.read(reinterpret_cast<char*>(&flags), sizeof(uint8_t));
				component.isTrigger = (flags & 1) != 0;
				break;
			}
			case ComponentID::GravitySource: {
				// -- Read GravitySource Component --
				auto& component = entity.addComponent<GravitySourceComponent>();
				uint32_t typeInt;
				in.read(reinterpret_cast<char*>(&typeInt), sizeof(uint32_t));
				component.type = static_cast<GravitySourceComponent::Type>(typeInt);
				in.read(reinterpret_cast<char*>(&component.strength), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.radius), sizeof(float));
				uint8_t flags;
				in.read(reinterpret_cast<char*>(&flags), sizeof(uint8_t));
				component.affectKinematic = (flags & 1) != 0;
				break;
			}
			case ComponentID::Script: {
				// -- Read C# Script Component --
				std::string className = readString(in);
				auto& component = entity.addComponent<ScriptComponent>(className);
				break;
			}
			case ComponentID::NativeScript: {
				// -- Read Native Script Component --
				std::string scriptName = readString(in);
				auto& component = entity.addComponent<NativeScriptComponent>();
				NativeScriptRegistry::bind(entity, scriptName);
				break;
			}
			case ComponentID::ParticleSystem: {
				// -- Read Particle System Component --
				auto& component = entity.addComponent<ParticleSystemComponent>();
				UUID textureUUID;
				in.read(reinterpret_cast<char*>(&textureUUID), sizeof(UUID));
				if (textureUUID.isValid()) component.texture = AssetManager::load<Texture2D>(textureUUID);
				in.read(reinterpret_cast<char*>(&component.sizeBegin), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.sizeEnd), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.lifeTime), sizeof(float));
				in.read(reinterpret_cast<char*>(&component.velocityVariation), sizeof(Vec3));
				in.read(reinterpret_cast<char*>(&component.colorBegin), sizeof(Vec4));
				in.read(reinterpret_cast<char*>(&component.colorEnd), sizeof(Vec4));
				break;
			}
			}

		}

		return entity;
	}

}
