#pragma once

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Components.h"

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

		void setParent(Entity parent) {
			auto& relationship = addOrGetComponent<RelationshipComponent>();
			relationship.parent = parent.m_handle;

			auto& parentRelationship = parent.addOrGetComponent<RelationshipComponent>();
			parentRelationship.children.push_back(m_handle);
		}

		Entity getParent() {
			if (hasComponent<RelationshipComponent>()) {
				entt::entity parentHandle = getComponent<RelationshipComponent>().parent;
				if (parentHandle != entt::null) {
					return Entity(parentHandle, m_scene);
				}
			}
			return Entity(entt::null, m_scene);
		}

		bool isValid() const { return m_handle != entt::null; }

		operator bool() const { return m_handle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_handle; }
		operator entt::entity() const { return m_handle; }
		bool operator ==(const Entity& other) const { return m_handle == other.m_handle && m_scene == other.m_scene; }
		bool operator !=(const Entity& other) const { return !(*this == other); }

	private:

		entt::entity m_handle{ entt::null };
		Scene* m_scene = nullptr;

		template<typename T>
		T& addOrGetComponent() {
			if (hasComponent<T>()) return getComponent<T>();
			return addComponent<T>();
		}

	};

}
