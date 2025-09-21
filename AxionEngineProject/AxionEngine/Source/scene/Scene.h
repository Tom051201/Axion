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

		// for rendering through a camera entity
		void onUpdate(Timestep ts);

		// for rendering through a custom camera
		void onUpdate(Timestep ts, const Camera& cam);

		void onEvent(Event& e);

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

		bool hasSkybox() const { return (bool)m_skybox; }
		void setSkybox(const AssetHandle<Skybox>& handle);
		void setSkyboxTexture(const std::string& crossPath);
		const std::string& getSkyboxPath() const { return m_skybox->getTexturePath(); }
		void removeSkybox();

	private:

		std::string m_title = "Untitled";
		entt::registry m_registry;

		std::vector<Entity> m_entitiesPendingDestroy;
		std::vector<std::function<void()>> m_componentsPendingRemove;

		Ref<Skybox> m_skybox; // TODO: add reference thru assethandle!
		AssetHandle<Skybox> m_skyboxHandle;
		bool m_setSkyboxRequested = false;
		AssetHandle<Skybox> m_requestedSky;

		bool onRenderingFinished(RenderingFinishedEvent& e);
		void flushDestroyedEntities();

		friend class Entity;
		friend class SceneSerializer;

	};

}
