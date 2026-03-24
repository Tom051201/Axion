#include "axpch.h"
#include "Scene.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/Renderer2D.h"
#include "AxionEngine/Source/render/Renderer3D.h"
#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/ScriptableEntity.h"
#include "AxionEngine/Source/audio/AudioManager.h"
#include "AxionEngine/Source/scene/ParticleSystem.h"
#include "AxionEngine/Source/physics/PhysicsSystem.h"
#include "AxionEngine/Source/scripting/ScriptEngine.h"

namespace Axion {

	Scene::Scene() {}

	Scene::~Scene() {
		release();
	}

	void Scene::release() {

		// -- Release Audio Components --
		auto acView = m_registry.view<AudioComponent>();
		for (auto entity : acView) {
			auto& ac = acView.get<AudioComponent>(entity);
			if (ac.audio != nullptr) { ac.audio->release(); }
		}

		// -- Release Native Script Components --
		auto nscView = m_registry.view<NativeScriptComponent>();
		for (auto entity : nscView) {
			auto& nsc = nscView.get<NativeScriptComponent>(entity);
			if (nsc.instance) {
				nsc.instance->onDestroy();
				nsc.destroyScript(&nsc);
			}
		}

		// -- Release C# Script Component --
		auto scView = m_registry.view<ScriptComponent>();
		for (auto entity : scView) {
			auto& sc = scView.get<ScriptComponent>(entity);
			if (sc.gcHandle) {
				ScriptEngine::destroyEntityScript(sc.gcHandle);
				sc.gcHandle = nullptr;
			}
		}

		// -- Reset Map --
		m_entityMap.clear();
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
		entt::entity handle = m_registry.create();
		Entity entity = { handle, this };
		m_entityMap[id] = handle;
		entity.addComponent<UUIDComponent>(id);
		entity.addComponent<TagComponent>(tag);
		entity.addComponent<TransformComponent>();
		return entity;
	}

	void Scene::destroyEntity(Entity entity) {
		if (!m_registry.all_of<PendingDestroyComponent>(entity)) {
			m_registry.emplace<PendingDestroyComponent>(entity);
		}
	}

	void Scene::flushDestroyedEntities() {
		auto destroyView = m_registry.view<PendingDestroyComponent>();
		for (auto e : destroyView) {
			Entity entity = { e, this };

			// -- Destroy Native Script --
			if (m_registry.all_of<NativeScriptComponent>(e)) {
				auto& nsc = m_registry.get<NativeScriptComponent>(e);
				if (nsc.instance) {
					nsc.instance->onDestroy();
					nsc.destroyScript(&nsc);
				}
			}

			// -- Destroy C# Script --
			if (m_registry.all_of<ScriptComponent>(e)) {
				auto& sc = m_registry.get<ScriptComponent>(e);
				if (sc.gcHandle) {
					ScriptEngine::destroyEntityScript(sc.gcHandle);
					sc.gcHandle = nullptr;
				}
			}

			// -- Remove From Map --
			if (m_registry.all_of<UUIDComponent>(e)) {
				auto& idc = m_registry.get<UUIDComponent>(e);
				m_entityMap.erase(idc.id);
			}

			// -- Remove Physics Actor --
			PhysicsSystem::destroyBody(entity);
		}

		// -- Destroy Entity --
		m_registry.destroy(destroyView.begin(), destroyView.end());

		// -- Remove components --
		for (auto& fn : m_componentsPendingRemove) {
			fn();
		}
		m_componentsPendingRemove.clear();
	}



	void Scene::onUpdateSimulation(Timestep ts, const Camera& cam) {
		// -- Physics --
		m_physicsAccumulator += ts.getSeconds();
		while (m_physicsAccumulator >= m_physicsTimeStep) {
			PhysicsSystem::step(this, m_physicsTimeStep);
			m_physicsAccumulator -= m_physicsTimeStep;
		}
		processPhysicsCallbacks();


		// -- Sync hierarchy for kinematic children --
		auto relView = m_registry.view<RelationshipComponent, TransformComponent, RigidBodyComponent>();
		for (auto [entity, relationship, transform, rb] : relView.each()) {
			if (relationship.parent != entt::null && rb.isKinematic) {
				Entity parentEntity = { relationship.parent, this };

				if (parentEntity.hasComponent<TransformComponent>()) {
					Mat4 worldTransform = getWorldTransform(Entity(entity, this));
					transform.position = worldTransform.getTranslation();
					transform.rotation = worldTransform.getRotation();
				}
			}
		}


		updateScripts(ts);

		onUpdate(ts, cam);
	}

	void Scene::onUpdate(Timestep ts) {
		// Physics
		m_physicsAccumulator += ts.getSeconds();
		while (m_physicsAccumulator >= m_physicsTimeStep) {
			PhysicsSystem::step(this, m_physicsTimeStep);
			m_physicsAccumulator -= m_physicsTimeStep;
		}
		processPhysicsCallbacks();


		updateScripts(ts);



		// -- Setup Camera --
		Camera* primaryCamera = nullptr;
		Mat4 cameraTransform;

		auto cameraView = m_registry.view<CameraComponent, TransformComponent>();
		for (auto [entity, camera, transform] : cameraView.each()) {
			if (camera.isPrimary) {
				primaryCamera = &camera.camera;
				cameraTransform = getWorldTransform({ entity, this });
				break;
			}
		}

		if (primaryCamera) {
			Mat4 view = cameraTransform.inverse();
			primaryCamera->setViewMatrix(view);

			onUpdate(ts, *primaryCamera);
		}

	}

	void Scene::onUpdate(Timestep ts, const Camera& cam) {

		// ----- Setup Scene Lighting -----
		LightingData lightData;
		lightData.ambientColor = { 0.03f, 0.03f, 0.03f, 1.0f }; // TODO: make selectable

		// -- Directional lights --
		auto dirLightView = m_registry.view<DirectionalLightComponent, TransformComponent>();
		for (auto [entity, dirLight, transform] : dirLightView.each()) {
			if (lightData.directionalLights.size() >= MAX_DIR_LIGHTS) break;

			Mat4 worldTransform = getWorldTransform({ entity, this });
			Vec4 forward = worldTransform * Vec4(0.0f, 0.0f, 1.0f, 0.0f);

			lightData.directionalLights.push_back({
				{ -forward.x, -forward.y, -forward.z },
				dirLight.color
			});
		}

		// -- Point lights --
		auto pointLightView = m_registry.view<PointLightComponent, TransformComponent>();
		for (auto [entity, pointLight, transform] : pointLightView.each()) {
			if (lightData.pointLights.size() >= MAX_POINT_LIGHTS) break;

			Mat4 worldTransform = getWorldTransform({ entity, this });

			lightData.pointLights.push_back({
				worldTransform.getTranslation(),
				pointLight.color * pointLight.intensity,
				pointLight.radius,
				pointLight.falloff
			});
		}

		// -- Spot lights --
		auto spotLightView = m_registry.view<SpotLightComponent, TransformComponent>();
		for (auto [entity, spotLight, transform] : spotLightView.each()) {
			if (lightData.spotLights.size() >= MAX_SPOT_LIGHTS) break;

			Mat4 worldTransform = getWorldTransform({ entity, this });
			Vec4 forward = worldTransform * Vec4(0.0f, 0.0f, 1.0f, 0.0f);

			lightData.spotLights.push_back({
				worldTransform.getTranslation(),
				{ forward.x, forward.y, forward.z },
				spotLight.color * spotLight.intensity,
				spotLight.range,
				std::cos(spotLight.innerConeAngle * 3.14159265f / 180.0f),
				std::cos(spotLight.outerConeAngle * 3.14159265f / 180.0f)
			});
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



		// ----- Pre-Calculate Batches -----
		std::unordered_map<AssetHandle<Mesh>, std::unordered_map<AssetHandle<Material>, std::vector<ObjectBuffer>>> renderBatches;
		auto meshRenderView = m_registry.view<MeshComponent, TransformComponent, MaterialComponent>();
		for (auto [entity, mesh, transform, material] : meshRenderView.each()) {
			if (mesh.handle.isValid() && material.handle.isValid()) {
				Ref<Material> matInstance = AssetManager::get<Material>(material.handle);
				if (!matInstance) continue;
				Mat4 worldTransform = getWorldTransform({entity, this});

				ObjectBuffer objData;
				objData.color = matInstance->getAlbedoColor().toFloat4(); // TODO: Move color out of ObjectBuffer if its strictly per material
				objData.modelMatrix = worldTransform.transposed().toXM();

				renderBatches[mesh.handle][material.handle].push_back(objData);
			}
		}

		// -- Shadow Map Pass --
		if (lightData.directionalLights.size() > 0) {
			Vec3 lightDir = lightData.directionalLights[0].direction;
			float lightDistance = 50.0f;
			float orthoSize = 100.0f;

			Vec3 lightUp = (std::abs(lightDir.y) > 0.99f) ? Vec3(0.0f, 0.0f, 1.0f) : Vec3(0.0f, 1.0f, 0.0f);
			Vec3 lightPos = lightDir * lightDistance;
			Mat4 lightView = Mat4::lookAt(lightPos, Vec3(0.0f, 0.0f, 0.0f), lightUp);
			Mat4 lightProjection = Mat4::orthographicOffCenter(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 100.0f);

			Renderer3D::beginScene(lightProjection, lightView.inverse());

			GraphicsContext::get()->bindDepthOnlyRenderTarget(Renderer::getShadowMap());

			for (auto& [meshHandle, materialMap] : renderBatches) {
				Ref<Mesh> mesh = AssetManager::get<Mesh>(meshHandle);
				if (!mesh) continue;

				std::vector<ObjectBuffer> flatInstanceData;
				for (auto& [matHandle, data] : materialMap) {
					flatInstanceData.insert(flatInstanceData.end(), data.begin(), data.end());
				}

				Renderer3D::drawMeshInstancedShadow(mesh, flatInstanceData);
			}

			GraphicsContext::get()->unbindDepthOnlyRenderTarget(Renderer::getShadowMap());
			Renderer3D::endScene();
		}

		Renderer::restoreRenderTarget();

		Renderer3D::beginScene(cam, lightData);


		// ----- Render Skybox -----
		if (m_skyboxHandle.isValid()) {
			Ref<Skybox> skybox = AssetManager::get<Skybox>(m_skyboxHandle);
			if (skybox) skybox->onUpdate(ts);
		}

		// -- Main Scene Pass --
		for (auto& [meshHandle, materialMap] : renderBatches) {
			Ref<Mesh> mesh = AssetManager::get<Mesh>(meshHandle);
			if (!mesh) continue;

			for (auto& [materialHandle, instanceData] : materialMap) {
				Ref<Material> mat = AssetManager::get<Material>(materialHandle);
				if (!mat) continue;

				Renderer3D::drawMeshInstanced(mesh, mat, instanceData);
			}
		}

		Renderer3D::endScene();
		Renderer2D::beginScene(cam);

		Mat4 viewMatrix = cam.getViewMatrix();

		// ----- Update and Render Particles -----
		auto particleView = m_registry.view<ParticleSystemComponent, TransformComponent>();
		for (auto [entity, particleSystem, transform] : particleView.each()) {

			for (ParticleProps& particle : particleSystem.particlePool) {
				if (!particle.active) continue;

				if (particle.lifeRemaining <= 0.0f) {
					particle.active = false;
					continue;
				}
				particle.lifeRemaining -= ts.getSeconds();

				particle.position += particle.velocity * ts.getSeconds();

				float lifePercentage = particle.lifeRemaining / particle.lifeTime;

				float currentSize = particle.sizeEnd + (particle.sizeBegin - particle.sizeEnd) * lifePercentage;

				Vec4 currentColor{
					particle.colorEnd.x + (particle.colorBegin.x - particle.colorEnd.x) * lifePercentage,
					particle.colorEnd.y + (particle.colorBegin.y - particle.colorEnd.y) * lifePercentage,
					particle.colorEnd.z + (particle.colorBegin.z - particle.colorEnd.z) * lifePercentage,
					particle.colorEnd.w + (particle.colorBegin.w - particle.colorEnd.w) * lifePercentage
				};

				if (particleSystem.texture.isValid()) {
					Renderer2D::drawBillboard(particle.position, { currentSize, currentSize }, viewMatrix, AssetManager::get<Texture2D>(particleSystem.texture), currentColor);
				}
				else {
					Renderer2D::drawBillboard(particle.position, { currentSize, currentSize }, viewMatrix, currentColor);
				}
			}
		}

		// ----- Render 2D sprites --
		auto spriteView = m_registry.view<SpriteComponent, TransformComponent>();
		for (auto [entity, sprite, transform] : spriteView.each()) {

			Mat4 worldTransform = getWorldTransform({ entity, this });
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

		Renderer2D::endScene();

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
			transform = transform * currentParent.getComponent<TransformComponent>().getTransform();
			currentParent = currentParent.getParent();
		}

		return transform;
	}

	Entity Scene::getEntityByUUID(UUID uuid) {
		if (m_entityMap.find(uuid) != m_entityMap.end()) {
			return { m_entityMap[uuid], this };
		}
		return {};
	}

	void Scene::queueCollision(entt::entity target, entt::entity other, const Vec3& contactPoint, const Vec3& contactNormal, const Vec3& impulse, bool isEnter) {
		m_collisionQueue.push_back({ target, other, contactPoint, contactNormal, impulse, isEnter });
	}

	void Scene::queueTrigger(entt::entity target, entt::entity other, bool isEnter) {
		m_triggerQueue.push_back({ target, other, isEnter });
	}

	void Scene::processPhysicsCallbacks() {

		// -- Process Collisions --
		for (auto& qc : m_collisionQueue) {
			Entity targetEntity = { qc.target, this };
			Entity otherEntity = { qc.other, this };

			Collision collisionData = { otherEntity, qc.contactPoint, qc.contactNormal, qc.impulse };

			if (qc.isEnter) {
				if (targetEntity.hasComponent<NativeScriptComponent>()) {
					auto& nsc = targetEntity.getComponent<NativeScriptComponent>();
					if (nsc.instance) nsc.instance->onCollisionEnter(collisionData);
				}
				if (targetEntity.hasComponent<ScriptComponent>()) {
					auto& sc = targetEntity.getComponent<ScriptComponent>();
					if (sc.isInstantiated) ScriptEngine::onCollisionEnter(sc.gcHandle, collisionData);
				}
			}
			else {
				if (targetEntity.hasComponent<NativeScriptComponent>()) {
					auto& nsc = targetEntity.getComponent<NativeScriptComponent>();
					if (nsc.instance) nsc.instance->onCollisionExit(collisionData);
				}
				if (targetEntity.hasComponent<ScriptComponent>()) {
					auto& sc = targetEntity.getComponent<ScriptComponent>();
					if (sc.isInstantiated) ScriptEngine::onCollisionExit(sc.gcHandle, collisionData);
				}
			}
		}
		m_collisionQueue.clear();

		// -- Process Triggers --
		for (auto& qt : m_triggerQueue) {
			Entity targetEntity = { qt.target, this };
			Entity otherEntity = { qt.other, this };

			if (qt.isEnter) {
				if (targetEntity.hasComponent<NativeScriptComponent>()) {
					auto& nsc = targetEntity.getComponent<NativeScriptComponent>();
					if (nsc.instance) nsc.instance->onTriggerEnter(otherEntity);
				}
			}
			else {
				if (targetEntity.hasComponent<NativeScriptComponent>()) {
					auto& nsc = targetEntity.getComponent<NativeScriptComponent>();
					if (nsc.instance) nsc.instance->onTriggerExit(otherEntity);
				}
			}
		}
		m_triggerQueue.clear();

	}

	void Scene::updateScripts(Timestep ts) {
		// -- Native Scripts --
		auto nativeView = m_registry.view<NativeScriptComponent>();
		for (auto entity : nativeView) {
			auto& nsc = nativeView.get<NativeScriptComponent>(entity);
			if (!nsc.instance) {
				nsc.instance = nsc.instantiateScript();
				nsc.instance->m_entity = Entity{ entity, this };
				nsc.instance->onCreate();
			}
			nsc.instance->onUpdate(ts);
		}

		// -- C# Scripts --
		ScriptEngine::setSceneContext(this);
		ScriptEngine::updateTime(ts.getSeconds());
		m_scriptEntitiesCache.clear();
		auto scriptView = m_registry.view<ScriptComponent>();
		for (auto e : scriptView) m_scriptEntitiesCache.push_back(e);
		for (auto e : m_scriptEntitiesCache) {
			if (!m_registry.valid(e) || !m_registry.all_of<ScriptComponent>(e)) continue;

			auto& sc = m_registry.get<ScriptComponent>(e);
			if (!sc.isInstantiated) {
				UUID entityID = m_registry.get<UUIDComponent>(e).id;
				sc.gcHandle = ScriptEngine::createEntityScript(entityID, sc.className.c_str());
				sc.isInstantiated = true;
			}
			if (sc.gcHandle) {
				ScriptEngine::updateEntityScript(sc.gcHandle, ts.getSeconds());
			}
		}
	}

}
