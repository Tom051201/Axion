#include "axpch.h"
#include "Scene.h"

#include "Axion/render/Renderer2D.h"

#include "Axion/scene/Components.h"
#include "Axion/scene/Entity.h"

namespace Axion {

	Scene::Scene() {
		m_uploadBuffer = ConstantBuffer::create(sizeof(ObjectBuffer));
	}

	Scene::~Scene() {
		m_uploadBuffer->release();
	}

	Entity Scene::createEntity() {
		return createEntity("Unnamed Entity");
	}

	Entity Scene::createEntity(const std::string& tag) {
		Entity entity = { m_registry.create(), this };
		entity.addComponent<TransformComponent>();
		entity.addComponent<TagComponent>(tag);
		return entity;
	}

	void Scene::onUpdate(Timestep ts) {
		Camera* primaryCamera = nullptr;
		Mat4* cameraTransform = nullptr;

		{
			auto group = m_registry.group<CameraComponent>(entt::get<TransformComponent>);
			for (auto entity : group) {
				auto& [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

				if (camera.isPrimary) {
					primaryCamera = &camera.camera;
					cameraTransform = &transform.transform;
					break;
				}

			}
		}

		if (primaryCamera) {
			Renderer2D::beginScene(primaryCamera->getProjection(), *cameraTransform);

			auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group) {
				auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::drawQuad(transform, sprite.color, m_uploadBuffer);
			}

			//Renderer2D::endScene();
		}

	}

}
