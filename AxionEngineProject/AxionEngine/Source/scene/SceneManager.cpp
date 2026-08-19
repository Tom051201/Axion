#include "axpch.h"
#include "SceneManager.h"

#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/SceneSerializer.h"

namespace Axion {

	struct SceneManagerData {
		Shared<Scene> scene;
		std::filesystem::path scenePath;
		bool isNewScene = false;
		std::function<void(Event&)> eventCallback;
		std::function<bool(RenderingFinishedEvent&)> onRenderingFinished;

		// -- New scene --
		bool newSceneRequest = false;

		// -- Load scene --
		bool loadSceneRequest = false;
		std::filesystem::path toLoadScenePath;

		// -- Save scene --
		bool saveSceneRequest = false;
		std::filesystem::path toSaveScenePath;

		// -- Unload scene --
		bool unloadSceneRequest = false;

		// --- Async Load State ---
		std::atomic<bool> isLoadingScene = false;
		std::atomic<bool> sceneLoadFinished = false;
		Shared<Scene> loadedSceneResult = nullptr;
		std::filesystem::path loadedScenePath;
	};

	static SceneManagerData* s_managerData;

	void SceneManager::initialize(std::function<void(Event&)> eventCallback) {
		s_managerData = new SceneManagerData();
		s_managerData->eventCallback = eventCallback;
		s_managerData->scene = std::make_shared<Scene>(); // load a blank scene
		s_managerData->onRenderingFinished = [&](RenderingFinishedEvent& e) {
			// -- Save scene --
			if (s_managerData->saveSceneRequest) {
				std::filesystem::path filePath = s_managerData->toSaveScenePath;
				if (!filePath.empty()) {
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

			// -- Load scene --
			if (s_managerData->loadSceneRequest) {
				std::filesystem::path filePath = s_managerData->toLoadScenePath;
				if (!filePath.empty() && std::filesystem::exists(filePath)) {
					Shared<Scene> scene = std::make_shared<Scene>();
					SceneSerializer serializer(scene);
					serializer.deserializeText(filePath);

					s_managerData->scenePath = filePath;
					s_managerData->isNewScene = false;
					setScene(scene);
					AX_CORE_LOG_INFO("Scene loaded");
				}
				else {
					AX_CORE_LOG_ERROR("Unable to load scene");
				}
				s_managerData->loadSceneRequest = false;
				s_managerData->toLoadScenePath.clear();
			}

			// -- Unload scene --
			if (s_managerData->unloadSceneRequest) {
				s_managerData->scenePath.clear();
				setScene(nullptr);
				AX_CORE_LOG_INFO("Unload scene");
				s_managerData->unloadSceneRequest = false;
			}

			// -- New Scene --
			if (s_managerData->newSceneRequest) {
				Shared<Scene> scene = std::make_shared<Scene>();
				s_managerData->isNewScene = true;
				s_managerData->scenePath.clear();
				setScene(scene);
				s_managerData->newSceneRequest = false;
				AX_CORE_LOG_INFO("New Scene");
			}

			// -- Check If Background Thread Finished Loading Scene --
			if (s_managerData->sceneLoadFinished) {
				if (s_managerData->loadedSceneResult) {
					s_managerData->scenePath = s_managerData->loadedScenePath;
					s_managerData->isNewScene = false;

					setScene(s_managerData->loadedSceneResult);
					AX_CORE_LOG_INFO("Scene safely loaded and swapped.");
				}
				else {
					AX_CORE_LOG_ERROR("Unable to load scene");
				}

				s_managerData->loadedSceneResult = nullptr;
				s_managerData->sceneLoadFinished = false;
				s_managerData->isLoadingScene = false;
			}

			return false;
		};
	}

	void SceneManager::shutdown() {
		s_managerData->scene->release();
		delete s_managerData;
	}

	void SceneManager::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<RenderingFinishedEvent>(s_managerData->onRenderingFinished);
	}

	void SceneManager::newScene() { s_managerData->newSceneRequest = true; }

	void SceneManager::loadScene(const std::filesystem::path& filePath) { // TODO: check here for loading bug
		if (s_managerData->isLoadingScene) return;

		s_managerData->loadSceneRequest = true;
		s_managerData->toLoadScenePath = filePath;

		// -- Spin up a background thread to parse the YAML ---
		std::thread([filePath]() {
			Shared<Scene> newScene = std::make_shared<Scene>();
			SceneSerializer serializer(newScene);

			bool success = false;
			if (std::filesystem::exists(filePath)) {
				success = serializer.deserializeText(filePath);
			}

			if (success) s_managerData->loadedSceneResult = newScene;
			else s_managerData->loadedSceneResult = nullptr;

			s_managerData->sceneLoadFinished = true;
		}).detach();
	}

	void SceneManager::saveScene(const std::filesystem::path& filePath) {
		s_managerData->toSaveScenePath = filePath;
		s_managerData->saveSceneRequest = true;
	}

	void SceneManager::unloadScene() {
		s_managerData->unloadSceneRequest = true;
	}

	Shared<Scene> SceneManager::getScene() { return s_managerData->scene; }

	bool SceneManager::hasScene() { return s_managerData && s_managerData->scene != nullptr; }

	bool SceneManager::isNewScene() { return s_managerData->isNewScene; }

	const std::filesystem::path& SceneManager::getScenePath() { return s_managerData->scenePath; }

	void SceneManager::setScene(const Shared<Scene>& scene) {
		s_managerData->scene = scene;

		AX_CORE_ASSERT(s_managerData->eventCallback, "Invalid event callback for scene manager");
		SceneChangedEvent ev;
		s_managerData->eventCallback(ev);
	}

	bool SceneManager::isLoadingScene() {
		return s_managerData ? s_managerData->isLoadingScene.load() : false;
	}

}
