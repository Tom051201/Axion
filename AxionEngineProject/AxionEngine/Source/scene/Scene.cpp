#include "axpch.h"
#include "Scene.h"

#include "AxionEngine/Source/render/Renderer2D.h"

#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/Entity.h"

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
			Renderer2D::beginScene(primaryCamera->getProjectionMatrix(), *cameraTransform);

			// 2D sprites
			//auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			//for (auto entity : group) {
			//	auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			//
			//	Renderer2D::drawQuad(transform.getTransform(), sprite.color, m_uploadBuffer);
			//}

			// material
			auto meshGroup = m_registry.group<TransformComponent>(entt::get<MaterialComponent>);
			for (auto entity : meshGroup) {
				auto& [transform, material] = meshGroup.get<TransformComponent, MaterialComponent>(entity);
			
				material.material->use();
				Renderer2D::drawQuad(transform.getTransform(), material.material, m_uploadBuffer);
			}

		}

	}

	void Scene::onUpdate(Timestep ts, const Camera& cam) {

		if (&cam) {
			Renderer2D::beginScene(cam);

			//auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			//for (auto entity : group) {
			//	auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			//
			//	Renderer2D::drawQuad(transform.getTransform(), sprite.color, m_uploadBuffer);
			//}

			// material
			auto meshGroup = m_registry.group<TransformComponent>(entt::get<MaterialComponent>);
			for (auto entity : meshGroup) {
				auto& [transform, material] = meshGroup.get<TransformComponent, MaterialComponent>(entity);
			
				material.material->use();
				Renderer2D::drawQuad(transform.getTransform(), material.material, m_uploadBuffer);
			}

		}
	}

}
