#pragma once

#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/render/Camera.h"

#include "AxionEngine/Vendor/entt/entt.hpp"


// TEMP
#include "AxionEngine/Source/render/Buffers.h"

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
