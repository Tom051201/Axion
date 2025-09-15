#include "axpch.h"
#include "SceneManager.h"

#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"

namespace Axion {

	struct SceneManagerData {
		Ref<Scene> scene;
		std::string scenePath;
		bool isNewScene = false;
		std::function<void(Event&)> eventCallback;
		std::function<bool(RenderingFinishedEvent&)> onRenderingFinished;
		// -- New scene --
		bool newSceneRequest = false;
		// -- Load scene --
		bool loadSceneRequest = false;
		std::string toLoadScenePath;
		// -- Save scene --
		bool saveSceneRequest = false;
		std::string toSaveScenePath;
	};

	static SceneManagerData* s_managerData;

	void SceneManager::initialize(std::function<void(Event&)> eventCallback) {
		s_managerData = new SceneManagerData();
		s_managerData->eventCallback = eventCallback;
		s_managerData->scene = std::make_shared<Scene>(); // load a blank scene
		s_managerData->onRenderingFinished = [&](RenderingFinishedEvent& e) {
			// -- New Scene --
			if (s_managerData->newSceneRequest) {
				Ref<Scene> scene = std::make_shared<Scene>();
				setScene(scene);
				s_managerData->isNewScene = true;
				s_managerData->newSceneRequest = false;
				s_managerData->scenePath.clear();
				AX_CORE_LOG_INFO("New Scene");
			}

			// -- Load scene --
			if (s_managerData->loadSceneRequest) {
				std::string filePath = s_managerData->toLoadScenePath;
				if (!filePath.empty() && std::filesystem::exists(std::filesystem::path(filePath))) {
					Ref<Scene> scene = std::make_shared<Scene>();
					SceneSerializer serializer(scene);
					serializer.deserializeText(filePath);

					setScene(scene);
					s_managerData->scenePath = filePath;
					s_managerData->isNewScene = false;
					AX_CORE_LOG_INFO("Scene loaded");
				}
				else {
					AX_CORE_LOG_ERROR("Unable to load scene");
				}
				s_managerData->loadSceneRequest = false;
				s_managerData->toLoadScenePath.clear();
			}

			// -- Save scene --
			if (s_managerData->saveSceneRequest) {
				std::string filePath = s_managerData->toSaveScenePath;
				if (!filePath.empty() && std::filesystem::exists(std::filesystem::path(filePath))) {
					SceneSerializer serializer(s_managerData->scene);
					serializer.serializeText(filePath);
					s_managerData->isNewScene = false;
					AX_CORE_LOG_INFO("Scene saved");
				}
				else {
					AX_CORE_LOG_ERROR("Unable to save scene");
				}
				s_managerData->saveSceneRequest = false;
				s_managerData->toSaveScenePath.clear();
			}

			return false;
		};
	}

	void SceneManager::release() {
		delete s_managerData;
	}

	void SceneManager::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<RenderingFinishedEvent>(s_managerData->onRenderingFinished);
	}

	void SceneManager::newScene() { s_managerData->newSceneRequest = true; }

	void SceneManager::loadScene(const std::string& filePath) {
		s_managerData->toLoadScenePath = filePath;
		s_managerData->loadSceneRequest = true;
	}

	void SceneManager::saveScene(const std::string& filePath) {
		s_managerData->toSaveScenePath = filePath;
		s_managerData->saveSceneRequest = true;
	}

	Ref<Scene> SceneManager::getScene() { return s_managerData->scene; }

	bool SceneManager::hasScene() { return s_managerData && s_managerData->scene != nullptr; }

	bool SceneManager::isNewScene() { return s_managerData->isNewScene; }

	const std::string& SceneManager::getScenePath() { return s_managerData->scenePath; }

	void SceneManager::setScene(const Ref<Scene>& scene) {
		s_managerData->scene = scene;

		AX_CORE_ASSERT(s_managerData->eventCallback, "Invalid event callback for scene manager");
		SceneChangedEvent ev;
		s_managerData->eventCallback(ev);
	}

}
