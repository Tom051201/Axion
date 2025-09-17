#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/scene/Scene.h"

#include <functional>

namespace Axion {

	class SceneManager {
	public:

		static void initialize(std::function<void(Event&)> eventCallback);
		static void release();
		static void onEvent(Event& e);

		static void newScene();
		static void loadScene(const std::string& filePath);
		static void saveScene(const std::string& filePath);
		static void unloadScene();
		static Ref<Scene> getScene();
		static bool hasScene();
		static bool isNewScene();
		static const std::string& getScenePath();

	private:

		static void setScene(const Ref<Scene>& scene);

	};

}
