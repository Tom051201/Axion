#pragma once

#include "AxionEngine/Source/scene/Entity.h"

#include "AxionEngine/Source/physics/PhysicsSystem.h"

namespace Axion {

	class ScriptableEntity {
	public:

		virtual ~ScriptableEntity() {}

		template<typename T>
		T& getComponent() {
			return m_entity.getComponent<T>();
		}

		virtual void onCreate() {}
		virtual void onDestroy() {}
		virtual void onUpdate(Timestep ts) {}

		virtual void onCollisionEnter(const Collision& collision) {}
		virtual void onCollisionExit(const Collision& collision) {}

		virtual void onTriggerEnter(Entity other) {}
		virtual void onTriggerExit(Entity other) {}

	private:

		Entity m_entity;

		friend class Scene;

	};

}
