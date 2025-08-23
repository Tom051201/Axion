#pragma once

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

	};

}
