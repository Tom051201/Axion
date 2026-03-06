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

		static Entity deserializeEntityNode(Scene* scene, YAML::Node& entityNode, bool generateNewUUID = false);

	private:

		Ref<Scene> m_scene;

		void serializeEntity(YAML::Emitter& out, Entity entity);

	};

}
