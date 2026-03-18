#pragma once

#include <string>

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"

namespace Axion::AAP {

	struct PrefabAssetData {
		UUID uuid;
		std::string name;
		Ref<Scene> scene;
		Entity entity;
	};

	class PrefabParser {
	public:

		static void createTextFile(const PrefabAssetData& data, const std::string& outputPath);
		static void createBinaryFile(const PrefabAssetData& data, const std::string& outputPath);

	};

}
