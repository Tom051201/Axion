#pragma once

#include <filesystem>

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionEngine/Source/scene/Scene.h"

namespace Axion {

	class SceneSerializer {
	public:

		SceneSerializer(const Ref<Scene>& scene);

		void serializeText(const std::filesystem::path& filePath, bool autoRegister = true);
		void serializeBinary(const std::filesystem::path& filePath);
		
		bool deserializeText(const std::filesystem::path& filePath);
		bool deserializeBinary(const std::filesystem::path& filePath);

		void serializeEntity(YAML::Emitter& out, Entity entity);
		static Entity deserializeEntityNode(Scene* scene, YAML::Node& entityNode, bool generateNewUUID = false);

		void serializeEntityBinary(std::ofstream& out, Entity entity);
		static Entity deserializeEntityBinary(Scene* scene, std::istream& in, bool generateNewUUID, std::vector<std::pair<Entity, UUID>>& relationshipsToBuild, uint32_t sceneVersion);

	private:

		Ref<Scene> m_scene;

	};

}
