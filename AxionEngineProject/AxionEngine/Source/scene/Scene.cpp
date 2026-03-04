#include "axpch.h"
#include "Scene.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/Renderer2D.h"
#include "AxionEngine/Source/render/Renderer3D.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/ScriptableEntity.h"
#include "AxionEngine/Source/audio/AudioManager.h"
#include "AxionEngine/Source/physics/PhysicsSystem.h"

namespace Axion {

	Scene::Scene() {}

	Scene::~Scene() {
		release();
	}

	void Scene::release() {
		{
			auto view = m_registry.view<AudioComponent>();
			for (auto entity : view) {
				auto& ac = view.get<AudioComponent>(entity);
				if (ac.audio != nullptr) { ac.audio->release(); }
			}
		}

		{
			auto view = m_registry.view<NativeScriptComponent>();
			for (auto entity : view) {
				auto& nsc = view.get<NativeScriptComponent>(entity);
				if (nsc.instance) {
					nsc.instance->onDestroy();
					nsc.destroyScript(&nsc);
				}
			}
		}

	}

	Entity Scene::createEntity() {
		return createEntityWithUUID("Unnamed Entity", UUID::generate());
	}

	Entity Scene::createEntity(const std::string& tag) {
		return createEntityWithUUID(tag, UUID::generate());
	}

	Entity Scene::createEntityWithUUID(UUID id) {
		return createEntityWithUUID("Unnamed Entity", id);
	}

	Entity Scene::createEntityWithUUID(const std::string& tag, UUID id) {
		Entity entity = { m_registry.create(), this };
		entity.addComponent<UUIDComponent>(id);
		entity.addComponent<TagComponent>(tag);
		entity.addComponent<TransformComponent>();
		return entity;
	}

	void Scene::destroyEntity(Entity entity) {
		m_entitiesPendingDestroy.push_back(entity);
	}

	void Scene::flushDestroyedEntities() {
		// -- Destroy native script --
		for (auto& e : m_entitiesPendingDestroy) {
			if (m_registry.all_of<NativeScriptComponent>(e)) {
				auto& nsc = m_registry.get<NativeScriptComponent>(e);
				if (nsc.instance) {
					nsc.instance->onDestroy();
					nsc.destroyScript(&nsc);
				}
			}
		}

		// -- Destroy entities --
		for (auto& e : m_entitiesPendingDestroy) {
			m_registry.destroy(e);
		}
		m_entitiesPendingDestroy.clear();

		// -- Remove components --
		for (auto& fn : m_componentsPendingRemove) {
			fn();
		}
		m_componentsPendingRemove.clear();
	}

	void Scene::onUpdateSimulation(Timestep ts, const Camera& cam) {
		// Physics
		m_physicsAccumulator += ts.getSeconds();
		while (m_physicsAccumulator >= m_physicsTimeStep) {
			PhysicsSystem::step(this, m_physicsTimeStep);
			m_physicsAccumulator -= m_physicsTimeStep;
		}

		// -- Sync hierarchy for kinematic children --
		auto group = m_registry.group<RelationshipComponent>(entt::get<TransformComponent, RigidBodyComponent>);
		for (auto entity : group) {
			auto& [relationship, transform, rb] = group.get<RelationshipComponent, TransformComponent, RigidBodyComponent>(entity);

			if (relationship.parent != entt::null && rb.isKinematic) {
				Entity parentEntity = { relationship.parent, this };

				if (parentEntity.hasComponent<TransformComponent>()) {
					Mat4 worldTransform = getWorldTransform(Entity(entity, this));
					transform.position = worldTransform.getTranslation();
					transform.rotation = worldTransform.getRotation();
				}
			}
		}

		onUpdate(ts, cam);
	}

	void Scene::onUpdate(Timestep ts) {
		// Physics
		m_physicsAccumulator += ts.getSeconds();
		while (m_physicsAccumulator >= m_physicsTimeStep) {
			PhysicsSystem::step(this, m_physicsTimeStep);
			m_physicsAccumulator -= m_physicsTimeStep;
		}

		

		Camera* primaryCamera = nullptr;
		Mat4 cameraTransform;

		// camera setup
		{
			auto group = m_registry.group<CameraComponent>(entt::get<TransformComponent>);
			for (auto entity : group) {
				auto& [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

				if (camera.isPrimary) {
					primaryCamera = &camera.camera;
					cameraTransform = transform.getTransform();
					break;
				}

			}
		}

		if (primaryCamera) {
			Mat4 view = cameraTransform.inverse();
			primaryCamera->setViewMatrix(view);

			onUpdate(ts, *primaryCamera);
		}

	}

	void Scene::onUpdate(Timestep ts, const Camera& cam) {
		if (&cam) {

			LightingData lightData;
			lightData.ambientColor = { 0.03f, 0.03f, 0.03f, 1.0f };

			// -- Directional lights --
			auto dirLightGroup = m_registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);
			for (auto e : dirLightGroup) {
				if (lightData.directionalLights.size() >= MAX_DIR_LIGHTS) break;

				auto& dlc = dirLightGroup.get<DirectionalLightComponent>(e);

				Mat4 worldTransform = getWorldTransform({ e, this });
				Vec4 forward = worldTransform * Vec4(0.0f, 0.0f, 1.0f, 0.0f);

				lightData.directionalLights.push_back({
					{ -forward.x, -forward.y, -forward.z },
					dlc.color
				});
			}

			// -- Point lights --
			auto pointLightGroup = m_registry.group<PointLightComponent>(entt::get<TransformComponent>);
			for (auto e : pointLightGroup) {
				if (lightData.pointLights.size() >= MAX_POINT_LIGHTS) break;

				auto& plc = pointLightGroup.get<PointLightComponent>(e);
				Mat4 worldTransform = getWorldTransform({ e, this });

				lightData.pointLights.push_back({
					worldTransform.getTranslation(),
					plc.color * plc.intensity,
					plc.radius,
					plc.falloff
				});
			}

			// -- Spot lights --
			auto spotLightGroup = m_registry.group<SpotLightComponent>(entt::get<TransformComponent>);
			for (auto e : spotLightGroup) {
				if (lightData.spotLights.size() >= MAX_SPOT_LIGHTS) break;

				auto& slc = spotLightGroup.get<SpotLightComponent>(e);

				Mat4 worldTransform = getWorldTransform({ e, this });
				Vec4 forward = worldTransform * Vec4(0.0f, 0.0f, 1.0f, 0.0f);

				lightData.spotLights.push_back({
					worldTransform.getTranslation(),
					{ forward.x, forward.y, forward.z },
					slc.color * slc.intensity,
					slc.range,
					std::cos(slc.innerConeAngle * 3.14159265f / 180.0f),
					std::cos(slc.outerConeAngle * 3.14159265f / 180.0f)
				});
			}

			Renderer3D::beginScene(cam, lightData);

			{
				auto view = m_registry.view<NativeScriptComponent>();
				for (auto entity : view) {
					auto& nsc = view.get<NativeScriptComponent>(entity);

					// -- Instantiate if it hasn't been yet --
					if (!nsc.instance) {
						nsc.instance = nsc.instantiateScript();
						nsc.instance->m_entity = Entity{ entity, this };
						nsc.instance->onCreate();
					}

					// -- Run the update --
					nsc.instance->onUpdate(ts);
				}
			}

			// ----- Spatial Audio Listener -----
			DirectX::XMFLOAT4X4 m;
			DirectX::XMStoreFloat4x4(&m, cam.getViewMatrix().inverse().toXM());
			Vec3 listenerPos(m._41, m._42, m._43);
			Vec3 listenerForward(-m._31, -m._32, -m._33);
			AudioManager::setListener(listenerPos, listenerForward);


			// ----- Spatial Audio Source ------
			auto view = m_registry.view<TransformComponent, AudioComponent>();
			view.each([&](auto entity, auto& transform, auto& audio) {
				if (audio.isSource && audio.audio != nullptr) {
					Ref<AudioSource> source = audio.audio;
					if (source) source->setPosition(transform.position);
				}
			});


			// ----- Render Skybox -----
			if (m_skyboxHandle.isValid()) {
				AssetManager::get<Skybox>(m_skyboxHandle)->onUpdate(ts);
			}


			// ----- Render Meshes -----
			std::unordered_map<AssetHandle<Mesh>, std::unordered_map<AssetHandle<Material>, std::vector<ObjectBuffer>>> renderBatches;

			auto group = m_registry.group<TransformComponent, MeshComponent, MaterialComponent>();
			for (auto e : group) {
				Entity entity = { e, this };

				auto& mesh = group.get<MeshComponent>(e);
				auto& material = group.get<MaterialComponent>(e);

				if (mesh.handle.isValid() && material.handle.isValid()) {
					Ref<Material> matInstance = AssetManager::get<Material>(material.handle);

					Mat4 worldTransform = getWorldTransform(entity);

					ObjectBuffer objData;
					objData.color = matInstance->getAlbedoColor().toFloat4(); // Optional: Move color out of ObjectBuffer if it's strictly per-material
					objData.modelMatrix = worldTransform.transposed().toXM();

					renderBatches[mesh.handle][material.handle].push_back(objData);
				}
			}

			// Flush the batches
			for (auto& [meshHandle, materialMap] : renderBatches) {
				Ref<Mesh> mesh = AssetManager::get<Mesh>(meshHandle);
				if (!mesh) continue;

				for (auto& [materialHandle, instanceData] : materialMap) {
					Ref<Material> mat = AssetManager::get<Material>(materialHandle);
					if (!mat) continue;

					Renderer3D::drawMeshInstanced(mesh, mat, instanceData);
				}
			}

			Renderer2D::beginScene(cam);
			auto spriteGroup = m_registry.group<SpriteComponent>(entt::get<TransformComponent>);
			for (auto e : spriteGroup) {
				auto& sprite = spriteGroup.get<SpriteComponent>(e);

				Mat4 worldTransform = getWorldTransform({ e, this });
				Vec3 worldPos = worldTransform.getTranslation();
				Vec3 worldScale = worldTransform.getScale();
				float worldRotZ = worldTransform.getRotation().toEulerAngles().z;

				if (sprite.texture.isValid()) {
					Renderer2D::drawQuad(
						worldPos,
						{ worldScale.x, worldScale.y },
						worldRotZ,
						AssetManager::get<Texture2D>(sprite.texture),
						sprite.tint
						);
				}
				else {
					Renderer2D::drawQuad(
						worldPos,
						{ worldScale.x, worldScale.y },
						worldRotZ,
						sprite.tint
					);
				}
			}

			Renderer::endScene();

		}
	}

	void Scene::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<RenderingFinishedEvent>(AX_BIND_EVENT_FN(onRenderingFinished));
	}

	bool Scene::onRenderingFinished(RenderingFinishedEvent& e) {
		flushDestroyedEntities();

		return false;
	}

	void Scene::setSkybox(const AssetHandle<Skybox>& handle) {
		m_skyboxHandle = handle;
	}

	void Scene::removeSkybox() {
		m_skyboxHandle.invalidate();
	}

	void Scene::onViewportResized(uint32_t width, uint32_t height) {
		auto view = m_registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& cc = view.get<CameraComponent>(entity);
			if (!cc.fixedAspectRatio) {
				cc.camera.setViewportSize(width, height);
			}
		}
	}

	void Scene::onPhysicsStart() {
		PhysicsSystem::onSceneStart(this);
	}

	void Scene::onPhysicsStop() {
		PhysicsSystem::onSceneStop(this);
	}

	Mat4 Scene::getWorldTransform(Entity entity) {
		Mat4 transform = entity.getComponent<TransformComponent>().getTransform();

		Entity currentParent = entity.getParent();
		while (currentParent) {
			transform = currentParent.getComponent<TransformComponent>().getTransform() * transform;
			currentParent = currentParent.getParent();
		}

		return transform;
	}

}
