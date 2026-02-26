#include "axpch.h"
#include "SceneSerializer.h"

#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/YamlHelper.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/render/Renderer3D.h"

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

		// -- SpriteComponent --
		if (entity.hasComponent<SpriteComponent>()) {
			out << YAML::Key << "SpriteComponent";
			out << YAML::BeginMap;
			auto& sprite = entity.getComponent<SpriteComponent>();
			if (sprite.texture.isValid()) {
				std::string relativeSpritePath = AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<Texture2D>(sprite.texture));
				out << YAML::Key << "Path" << YAML::Value << relativeSpritePath;
			}
			else {
				out << YAML::Key << "Path" << YAML::Value << "None";
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
				std::string relativeMatPath = AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<Material>(mat.handle));
				out << YAML::Key << "Path" << YAML::Value << relativeMatPath;
			}
			else {
				out << YAML::Key << "Path" << YAML::Value << "None";
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
				out << YAML::Key << "AudioSource" << YAML::BeginMap; // AudioSource
				std::string clipPath = AssetManager::getRelativeToAssets(AssetManager::getAssetFilePath<AudioClip>(ac.audio->getClipHandle()));
				out << YAML::Key << "AudioClip" << YAML::Value << clipPath;
				out << YAML::Key << "Volume" << YAML::Value << ac.audio->getVolume();
				out << YAML::Key << "Pitch" << YAML::Value << ac.audio->getPitch();
				out << YAML::Key << "Pan" << YAML::Value << ac.audio->getPan();
				out << YAML::Key << "IsSpatial" << YAML::Value << ac.audio->isSpatial();
				out << YAML::Key << "Position" << YAML::Value << ac.audio->getPosition();
				out << YAML::Key << "Velocity" << YAML::Value << ac.audio->getVelocity();
				out << YAML::Key << "MinDistance" << YAML::Value << ac.audio->getMinDistance();
				out << YAML::Key << "MaxDistance" << YAML::Value << ac.audio->getMaxDistance();
				out << YAML::Key << "DopplerFactor" << YAML::Value << ac.audio->getDopplerFactor();
				out << YAML::EndMap; // AudioSource
			}
			else {
				out << YAML::Key << "AudioSource" << YAML::Value << "None";
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
			out << YAML::Key << "StaticFriction" << YAML::Value << bcc.staticFriction;
			out << YAML::Key << "DynamicFriction" << YAML::Value << bcc.dynamicFriction;
			out << YAML::Key << "Restitution" << YAML::Value << bcc.restitution;
			out << YAML::EndMap;
		}

		// -- SphereColliderComponent --
		if (entity.hasComponent<SphereColliderComponent>()) {
			out << YAML::Key << "SphereColliderComponent";
			out << YAML::BeginMap;
			auto& scc = entity.getComponent<SphereColliderComponent>();
			out << YAML::Key << "Radius" << YAML::Value << scc.radius;
			out << YAML::Key << "Offset" << YAML::Value << scc.offset;
			out << YAML::Key << "StaticFriction" << YAML::Value << scc.staticFriction;
			out << YAML::Key << "DynamicFriction" << YAML::Value << scc.dynamicFriction;
			out << YAML::Key << "Restitution" << YAML::Value << scc.restitution;
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
			out << YAML::Key << "StaticFriction" << YAML::Value << ccc.staticFriction;
			out << YAML::Key << "DynamicFriction" << YAML::Value << ccc.dynamicFriction;
			out << YAML::Key << "Restitution" << YAML::Value << ccc.restitution;
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

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::serializeText(const std::string& absoluteFilePath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		// -- Scene name --
		out << YAML::Key << "Scene" << YAML::Value << m_scene->getTitle();

		// -- Skybox --
		if (m_scene->m_skyboxHandle.isValid()) {
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

				// -- SpriteComponent --
				auto spriteComponent = entity["SpriteComponent"];
				if (spriteComponent) {
					auto& sc = deserializedEntity.addComponent<SpriteComponent>();
					std::string relPath = spriteComponent["Path"].as<std::string>();
					if (relPath != "None") {
						std::string absPath = AssetManager::getAbsolute(relPath);
						AssetHandle<Texture2D> handle = AssetManager::load<Texture2D>(absPath);
						sc.texture = handle;
					}
					else {
						sc.texture = AssetHandle<Texture2D>();
					}
					sc.tint = spriteComponent["Tint"].as<Vec4>();
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

				// -- AudioComponent --
				auto audioComponent = entity["AudioComponent"];
				if (audioComponent) {
					auto& ac = deserializedEntity.addComponent<AudioComponent>();
					ac.isListener = audioComponent["IsListener"].as<bool>();
					ac.isSource = audioComponent["IsSource"].as<bool>();

					auto as = audioComponent["AudioSource"];
					if (as && as.IsScalar() && as.as<std::string>() == "None") {
						ac.audio = nullptr;
					}
					else {
						auto as = audioComponent["AudioSource"];
						std::string clipPath = AssetManager::getAbsolute(as["AudioClip"].as<std::string>());
						AssetHandle<AudioClip> handle = AssetManager::load<AudioClip>(clipPath);
						ac.audio = std::make_shared<AudioSource>(handle);
						ac.audio->setVolume(as["Volume"].as<float>());
						ac.audio->setPitch(as["Pitch"].as<float>());
						ac.audio->setPan(as["Pan"].as<float>());
						if (as["IsSpatial"].as<bool>()) {
							ac.audio->enableSpatial();
						} else {
							ac.audio->disableSpatial();
						}
						ac.audio->setPosition(as["Position"].as<Vec3>());
						ac.audio->setVelocity(as["Velocity"].as<Vec3>());
						ac.audio->setMinDistance(as["MinDistance"].as<float>());
						ac.audio->setMaxDistance(as["MaxDistance"].as<float>());
						ac.audio->setDopplerFactor(as["DopplerFactor"].as<float>());
					}
				}

				// -- CameraComponent --
				auto cameraComponent = entity["CameraComponent"];
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
				auto directionalLightComponent = entity["DirectionalLightComponent"];
				if (directionalLightComponent) {
					auto& dlc = deserializedEntity.addComponent<DirectionalLightComponent>();
					dlc.color = directionalLightComponent["Color"].as<Vec4>();
				}

				// -- PointLightComponent --
				auto pointLightComponent = entity["PointLightComponent"];
				if (pointLightComponent) {
					auto& plc = deserializedEntity.addComponent<PointLightComponent>();
					plc.color = pointLightComponent["Color"].as<Vec4>();
					plc.intensity = pointLightComponent["Intensity"].as<float>();
					plc.radius = pointLightComponent["Radius"].as<float>();
					plc.falloff = pointLightComponent["Falloff"].as<float>();
				}

				// -- SpotLightComponent --
				auto spotLightComponent = entity["SpotLightComponent"];
				if (spotLightComponent) {
					auto& slc = deserializedEntity.addComponent<SpotLightComponent>();
					slc.color = spotLightComponent["Color"].as<Vec4>();
					slc.intensity = spotLightComponent["Intensity"].as<float>();
					slc.range = spotLightComponent["Range"].as<float>();
					slc.innerConeAngle = spotLightComponent["InnerConeAngle"].as<float>();
					slc.outerConeAngle = spotLightComponent["OuterConeAngle"].as<float>();
				}

				// -- RigidBodyComponent --
				auto rigidBodyComponent = entity["RigidBodyComponent"];
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
				auto boxColliderComponent = entity["BoxColliderComponent"];
				if (boxColliderComponent) {
					auto& bcc = deserializedEntity.addComponent<BoxColliderComponent>();
					bcc.halfExtents = boxColliderComponent["HalfExtents"].as<Vec3>();
					bcc.offset = boxColliderComponent["Offset"].as<Vec3>();
					bcc.staticFriction = boxColliderComponent["StaticFriction"].as<float>();
					bcc.dynamicFriction = boxColliderComponent["DynamicFriction"].as<float>();
					bcc.restitution = boxColliderComponent["Restitution"].as<float>();
				}

				// -- SphereColliderComponent --
				auto sphereColliderComponent = entity["SphereColliderComponent"];
				if (sphereColliderComponent) {
					auto& scc = deserializedEntity.addComponent<SphereColliderComponent>();
					scc.radius = sphereColliderComponent["Radius"].as<float>();
					scc.offset = sphereColliderComponent["Offset"].as<Vec3>();
					scc.staticFriction = sphereColliderComponent["StaticFriction"].as<float>();
					scc.dynamicFriction = sphereColliderComponent["DynamicFriction"].as<float>();
					scc.restitution = sphereColliderComponent["Restitution"].as<float>();
				}

				// -- CapsuleColliderComponent --
				auto capsuleColliderComponent = entity["CapsuleColliderComponent"];
				if (capsuleColliderComponent) {
					auto& ccc = deserializedEntity.addComponent<CapsuleColliderComponent>();
					ccc.radius = capsuleColliderComponent["Radius"].as<float>();
					ccc.halfHeight = capsuleColliderComponent["HalfHeight"].as<float>();
					ccc.offset = capsuleColliderComponent["Offset"].as<Vec3>();
					ccc.staticFriction = capsuleColliderComponent["StaticFriction"].as<float>();
					ccc.dynamicFriction = capsuleColliderComponent["DynamicFriction"].as<float>();
					ccc.restitution = capsuleColliderComponent["Restitution"].as<float>();
				}

				// -- GravitySourceComponent --
				auto gravitySourceComponent = entity["GravitySourceComponent"];
				if (gravitySourceComponent) {
					auto& gsc = deserializedEntity.addComponent<GravitySourceComponent>();
					gsc.type = EnumUtils::gravitySourceTypeFromString(gravitySourceComponent["Type"].as<std::string>());
					gsc.strength = gravitySourceComponent["Strength"].as<float>();
					gsc.radius = gravitySourceComponent["Radius"].as<float>();
					gsc.affectKinematic = gravitySourceComponent["AffectKinematic"].as<bool>();
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
