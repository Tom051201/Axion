#pragma once

#include <functional>
#include <filesystem>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/scene/Scene.h"

namespace Axion {

	class SceneManager {
	public:

		static void initialize(std::function<void(Event&)> eventCallback);
		static void shutdown();
		static void onEvent(Event& e);

		static void setScene(const Ref<Scene>& scene);
		static void newScene();
		static void loadScene(const std::filesystem::path& filePath);
		static void saveScene(const std::filesystem::path& filePath);
		static void unloadScene();
		static Ref<Scene> getScene();
		static bool hasScene();
		static bool isNewScene();
		static const std::filesystem::path& getScenePath();

	};

}
