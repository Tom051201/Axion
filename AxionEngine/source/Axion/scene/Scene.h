#pragma once

#include "Axion/core/Timestep.h"
#include "Axion/render/Camera.h"

#include "entt.hpp"


// TEMP
#include "Axion/render/Buffers.h"

namespace Axion {

	class Entity;

	class Scene {
	public:
		
		Scene();
		~Scene();

		void onUpdate(Timestep ts);
		void onUpdate(Timestep ts, const Camera& cam, const Mat4& transform);

		Entity createEntity();
		Entity createEntity(const std::string& tag);

	private:

		entt::registry m_registry;
		
		friend class Entity;
		
		
		// TEMP
		Ref<ConstantBuffer> m_uploadBuffer;
	};

}
