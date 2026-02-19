#include "axpch.h"
#include "Scene.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/Renderer2D.h"
#include "AxionEngine/Source/render/Renderer3D.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/audio/AudioManager.h"

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

	void Scene::onUpdate(Timestep ts) {
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
			lightData.direction = { 0.5f, 1.0f, -0.5f };
			lightData.color = Vec4::one();
			
			auto lightView = m_registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);
			for (auto entity : lightView) {
				auto& [transform, dlc] = lightView.get<TransformComponent, DirectionalLightComponent>(entity);

				Mat4 transformMat = transform.getTransform();
				Vec4 forward = transformMat * Vec4(0.0f, 0.0f, 1.0f, 0.0f);

				lightData.direction = { -forward.x, -forward.y, -forward.z };
				lightData.color = dlc.color;

				break;
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
			auto group = m_registry.group<TransformComponent, MeshComponent, MaterialComponent, ConstantBufferComponent>();
			for (auto entity : group) {
				auto& transform = group.get<TransformComponent>(entity);
				auto& mesh = group.get<MeshComponent>(entity);
				auto& material = group.get<MaterialComponent>(entity);
				auto& cb = group.get<ConstantBufferComponent>(entity);

				if (mesh.handle.isValid() && material.handle.isValid() && cb.uploadBuffer != nullptr) {
					Renderer3D::drawMesh(
						transform.getTransform(),
						AssetManager::get<Mesh>(mesh.handle),
						AssetManager::get<Material>(material.handle),
						cb.uploadBuffer
					);
				}
			}

			Renderer2D::beginScene(cam);
			auto spriteGroup = m_registry.group<SpriteComponent>(entt::get<TransformComponent>);
			for (auto entity : spriteGroup) {
				auto& transform = spriteGroup.get<TransformComponent>(entity);
				auto& sprite = spriteGroup.get<SpriteComponent>(entity);

				Renderer2D::drawQuad(
					transform.position,
					{ transform.scale.x, transform.scale.y },
					transform.rotation.z,
					sprite.tint
				);
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

}
