#pragma once

#include <string>
#include <unordered_map>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/graphics/Texture.h"

namespace Axion {

	class EditorResourceManager {
	public:

		static void initialize();
		static void shutdown();

		static Ref<Texture2D> getIcon(const std::string& name);
		static void loadIcon(const std::string& name, const std::string& path);

	private:

		static std::unordered_map<std::string, Ref<Texture2D>> s_texture2Ds;

	};

}
