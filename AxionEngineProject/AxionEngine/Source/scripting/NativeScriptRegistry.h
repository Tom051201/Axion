#pragma once

#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/core/Core.h"

#include <unordered_map>
#include <string>
#include <functional>

namespace Axion {

	class NativeScriptRegistry {
	public:

		using BindFunction = std::function<void(Entity)>;

		template<typename T>
		static void registerScript(const std::string& name) {
			s_registry[name] = [name](Entity entity) {
				entity.getComponent<NativeScriptComponent>().bind<T>(name);
			};
		}

		static void bind(Entity entity, const std::string& name) {
			if (s_registry.find(name) != s_registry.end()) {
				s_registry[name](entity);
			}
			else {
				AX_CORE_LOG_WARN("Native script not registered: {}", name);
			}
		}

	private:

		inline static std::unordered_map<std::string, BindFunction> s_registry;

	};

}
