#pragma once

#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/events/RenderingEvent.h"
#include "AxionEngine/Source/scene/Skybox.h"

#include "AxionEngine/Vendor/entt/entt.hpp"

namespace Axion {

	class Entity;

	class Scene {
	public:

		Scene();
		~Scene();

		void release();

		// for rendering through a camera entity
		void onUpdate(Timestep ts);

		// for rendering through a custom camera
		void onUpdate(Timestep ts, const Camera& cam);

		void onEvent(Event& e);

		void onPhysicsStart();
		void onPhysicsStop();

		Entity createEntity();
		Entity createEntity(const std::string& tag);
		Entity createEntityWithUUID(UUID id);
		Entity createEntityWithUUID(const std::string& tag, UUID id);

		// will add Entity or Component to destroy queue
		void destroyEntity(Entity entity);
		template<typename T>
		void removeComponent(Entity entity) {
			m_componentsPendingRemove.push_back([this, entity]() {
				if (m_registry.all_of<T>(entity)) {
					m_registry.remove<T>(entity);
				}
			});
		}

		entt::registry& getRegistry() { return m_registry; }

		void setTitle(const std::string& title) { m_title = title; }
		const std::string& getTitle() const { return m_title; }

		void setSkybox(const AssetHandle<Skybox>& handle);
		AssetHandle<Skybox> getSkyboxHandle() const { return m_skyboxHandle; }
		bool hasSkybox() const { return m_skyboxHandle.isValid(); }
		void removeSkybox();

		void onViewportResized(uint32_t width, uint32_t height);

	private:

		std::string m_title = "Untitled";
		entt::registry m_registry;

		std::vector<Entity> m_entitiesPendingDestroy;
		std::vector<std::function<void()>> m_componentsPendingRemove;

		AssetHandle<Skybox> m_skyboxHandle;

		float m_physicsAccumulator = 0.0f;
		const float m_physicsTimeStep = 1.0f / 60.0f; // 60 FPS Physics

		bool onRenderingFinished(RenderingFinishedEvent& e);
		void flushDestroyedEntities();

		friend class Entity;
		friend class SceneSerializer;

	};

}
