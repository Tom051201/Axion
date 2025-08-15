#pragma once

#include "AxionEngine/Source/scene/Scene.h"

#include "AxionEngine/Vendor/entt/entt.hpp"

namespace Axion {

	class Entity {
	public:
		
		Entity() = default;
		Entity(entt::entity handle, Scene* scene) : m_handle(handle), m_scene(scene) {}
		Entity(const Entity& other) = default;
		~Entity() = default;

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) {
			AX_CORE_ASSERT(!hasComponent<T>(), "Entity already has component!");
			return m_scene->m_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
		}

		template<typename T>
		void removeComponent() {
			AX_CORE_ASSERT(hasComponent<T>(), "Entity does not have component!");
			m_scene->m_registry.remove<T>(m_handle);
		}

		template<typename T>
		T& getComponent() {
			AX_CORE_ASSERT(hasComponent<T>(), "Entity does not have component!");
			return m_scene->m_registry.get<T>(m_handle);
		}

		template<typename T>
		bool hasComponent() {
			return m_scene->m_registry.all_of<T>(m_handle);
		}

		operator bool() const { return m_handle != (entt::entity)0; }
		operator uint32_t() const { return (uint32_t)m_handle; }
		bool operator ==(const Entity& other) const { return m_handle == other.m_handle && m_scene == other.m_scene; }
		bool operator !=(const Entity& other) const { return !(*this == other); }

	private:

		entt::entity m_handle{ entt::null };
		Scene* m_scene = nullptr;

	};

}
