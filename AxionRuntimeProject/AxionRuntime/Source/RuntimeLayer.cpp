#include "RuntimeLayer.h"

namespace Axion {

	RuntimeLayer::RuntimeLayer() : Layer("RuntimeLayer") {}

	RuntimeLayer::~RuntimeLayer() {}

	void RuntimeLayer::onAttach() {
		ProjectManager::loadRuntimeProject();

		Ref<Project> project = ProjectManager::getProject();
		if (!project) return;

		Application::get().setWindowTitle(project->getName());
		if (!project->getAppIconPath().empty()) {
			Application::get().setWindowIcon(project->getAppIconPath());
		}

		m_activeScene = std::make_shared<Scene>();
		UUID sceneUUID = project->getDefaultSceneUUID();

		if (sceneUUID.isValid() && project->getAssetRegistry()->contains(sceneUUID)) {
			std::string sceneBinaryPath = project->getAssetRegistry()->get(sceneUUID).filePath.string();

			SceneSerializer serializer(m_activeScene);
			if (serializer.deserializeBinary(sceneBinaryPath)) {
				SceneManager::setScene(m_activeScene);

				uint32_t width = Application::get().getWindow().getWidth();
				uint32_t height = Application::get().getWindow().getHeight();
				m_activeScene->onViewportResized(width, height);

				m_activeScene->onPhysicsStart();
			}
			else {
				AX_CORE_LOG_ERROR("Runtime failed to deserialize default scene");
			}
		}
		else {
			AX_CORE_LOG_WARN("Default scene UUID is invalid or missing from Asset Registry.");
		}

	}

	void RuntimeLayer::onDetach() {
		m_activeScene->onPhysicsStop();
	}

	void RuntimeLayer::onUpdate(Timestep ts) {
		m_activeScene->onUpdate(ts);
	}

	void RuntimeLayer::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowResizeEvent>(AX_BIND_EVENT_FN(RuntimeLayer::onWindowResize));
	}

	bool RuntimeLayer::onWindowResize(WindowResizeEvent& e) {
		if (m_activeScene && e.getWidth() > 0 && e.getHeight() > 0) {
			m_activeScene->onViewportResized(e.getWidth(), e.getHeight());
		}

		return false;
	}

}
