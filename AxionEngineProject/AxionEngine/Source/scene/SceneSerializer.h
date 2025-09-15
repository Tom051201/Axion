#pragma once

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/scene/Scene.h"

namespace Axion {

	class SceneSerializer {
	public:

		SceneSerializer(const Ref<Scene>& scene);

		void serializeText(const std::string& filePath);
		void serializeBinary(const std::string& filePath);
		
		bool deserializeText(const std::string& filePath);
		bool deserializeBinary(const std::string& filePath);

	private:

		Ref<Scene> m_scene;

		static void serializeEntity(YAML::Emitter& out, Entity entity);

		static std::string getRelAssetPath(const std::string& path);
		static std::string getAbsAssetPath(const std::string& path);

	};

}
