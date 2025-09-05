#include "axpch.h"
#include "Scene.h"

#include "AxionEngine/Source/render/Renderer2D.h"
#include "AxionEngine/Source/render/Renderer3D.h"

#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/Entity.h"

namespace Axion {

	Scene::Scene() {}

	Scene::~Scene() {}

	Entity Scene::createEntity() {
		return createEntityWithUUID("Unnamed Entity", UUID());
	}

	Entity Scene::createEntity(const std::string& tag) {
		return createEntityWithUUID(tag, UUID());
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
		// destroy entities
		for (auto& e : m_entitiesPendingDestroy) {
			m_registry.destroy(e);
		}
		m_entitiesPendingDestroy.clear();

		// remove components
		for (auto& fn : m_componentsPendingRemove) {
			fn();
		}
		m_componentsPendingRemove.clear();

	}

	void Scene::onUpdate(Timestep ts) {
		Camera* primaryCamera = nullptr;
		Mat4* cameraTransform = nullptr;

		// camera setup
		{
			auto group = m_registry.group<CameraComponent>(entt::get<TransformComponent>);
			for (auto entity : group) {
				auto& [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

				if (camera.isPrimary) {
					primaryCamera = &camera.camera;
					cameraTransform = &transform.getTransform();
					break;
				}

			}
		}

		if (primaryCamera) {
			Renderer3D::beginScene(primaryCamera->getProjectionMatrix(), *cameraTransform);

			auto group = m_registry.group<TransformComponent, MeshComponent, MaterialComponent, ConstantBufferComponent>();
			for (auto entity : group) {
				auto& transform = group.get<TransformComponent>(entity);
				auto& mesh = group.get<MeshComponent>(entity);
				auto& material = group.get<MaterialComponent>(entity);
				auto& cb = group.get<ConstantBufferComponent>(entity);

				if (mesh.mesh != nullptr && material.material != nullptr && cb.uploadBuffer != nullptr) {
					Renderer3D::drawMesh(transform.getTransform(), mesh.mesh, material.material, cb.uploadBuffer);
				}
			}

		}

	}

	void Scene::onUpdate(Timestep ts, const Camera& cam) {

		if (&cam) {
			Renderer3D::beginScene(cam);

			// ----- Render Skybox -----
			if (m_skybox != nullptr) {
				m_skybox->onUpdate(ts);
			}


			// ----- Render Meshes -----
			auto group = m_registry.group<TransformComponent, MeshComponent, MaterialComponent, ConstantBufferComponent>();
			for (auto entity : group) {
				auto& transform = group.get<TransformComponent>(entity);
				auto& mesh = group.get<MeshComponent>(entity);
				auto& material = group.get<MaterialComponent>(entity);
				auto& cb = group.get<ConstantBufferComponent>(entity);
			
				if (mesh.mesh != nullptr && material.material != nullptr && cb.uploadBuffer != nullptr) {
					Renderer3D::drawMesh(transform.getTransform(), mesh.mesh, material.material, cb.uploadBuffer);
				}
			}

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

	void Scene::setSkyboxTexture(const std::string& crossPath) {
		m_skybox->setTexture(crossPath);
	}

}
