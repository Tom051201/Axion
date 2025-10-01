#include "axpch.h"
#include "ProjectManager.h"

#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/events/RenderingEvent.h"
#include "AxionEngine/Source/project/Project.h"
#include "AxionEngine/Source/scene/SceneManager.h"

namespace Axion {

	struct ProjectManagerData {
		Ref<Project> project = nullptr;
		std::string projectPath;
		std::function<void(Event&)> eventCallback;
		std::function<bool(RenderingFinishedEvent&)> onRenderingFinished;
		// -- New project --
		bool newProjectRequest = false;
		ProjectSpecification newProjectSpecification;
		// -- Load project --
		bool loadProjectRequest = false;
		std::string toLoadProjectPath;
		// -- Save project --
		bool saveProjectRequest = false;
		std::string toSaveProjectPath;
		// -- Unload project --
		bool unloadProjectRequest = false;
	};

	static ProjectManagerData* s_managerData;

	void ProjectManager::initialize(std::function<void(Event&)> eventCallback) {
		s_managerData = new ProjectManagerData();
		s_managerData->eventCallback = eventCallback;
		s_managerData->onRenderingFinished = [&](RenderingFinishedEvent& e) {
			// -- Save project --
			if (s_managerData->saveProjectRequest) {
				std::string filePath = s_managerData->toSaveProjectPath;
				if (!filePath.empty() && std::filesystem::exists(std::filesystem::path(filePath))) {
					s_managerData->project->save(filePath);
					AX_CORE_LOG_INFO("Project saved");
				}
				else {
					AX_CORE_LOG_ERROR("Unable to save project");
				}
				s_managerData->saveProjectRequest = false;
				s_managerData->toSaveProjectPath.clear();
			}

			// -- Load project --
			if (s_managerData->loadProjectRequest) {
				std::string filePath = s_managerData->toLoadProjectPath;
				if (!filePath.empty() && std::filesystem::exists(std::filesystem::path(filePath))) {
					setProject(Project::load(filePath));
					s_managerData->projectPath = filePath;

					if (!s_managerData->project->getDefaultScene().empty()) {
						SceneManager::loadScene(s_managerData->project->getDefaultScene());
					}
					else {
						SceneManager::newScene();
					}

					AX_CORE_LOG_INFO("Project loaded");
				}
				else {
					AX_CORE_LOG_ERROR("Unable to load project");
				}
				s_managerData->loadProjectRequest = false;
				s_managerData->toLoadProjectPath.clear();
			}

			// -- Unload project --
			if (s_managerData->unloadProjectRequest) {
				setProject(nullptr);
				s_managerData->projectPath.clear();
				AX_CORE_LOG_INFO("Unloaded project");
				s_managerData->unloadProjectRequest = false;
			}

			// -- New project --
			if (s_managerData->newProjectRequest) {
				Project::createNew(s_managerData->newProjectSpecification);
				AX_CORE_LOG_INFO("New Project");
				s_managerData->newProjectRequest = false;
				s_managerData->newProjectSpecification = {};
			}

			return false;
		};
	}

	void ProjectManager::release() {
		delete s_managerData;
	}

	void ProjectManager::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<RenderingFinishedEvent>(s_managerData->onRenderingFinished);
	}

	void ProjectManager::newProject(const ProjectSpecification& spec) {
		s_managerData->newProjectSpecification = spec;
		s_managerData->newProjectRequest = true;
	}

	void ProjectManager::loadProject(const std::string& filePath) {
		s_managerData->toLoadProjectPath = filePath;
		s_managerData->loadProjectRequest = true;
	}

	void ProjectManager::saveProject(const std::string& filePath) {
		s_managerData->toSaveProjectPath = filePath;
		s_managerData->saveProjectRequest = true;
	}

	void ProjectManager::unloadProject() {
		s_managerData->unloadProjectRequest = true;
	}

	Ref<Project> ProjectManager::getProject() { return s_managerData->project; }

	bool ProjectManager::hasProject() { return s_managerData && s_managerData->project != nullptr; }

	const std::string& ProjectManager::getProjectFilePath() { return s_managerData->projectPath; }

	void ProjectManager::setProject(const Ref<Project>& project) {
		s_managerData->project = project;
		AX_CORE_ASSERT(s_managerData->eventCallback, "Invalid event callback for project manager")
		ProjectChangedEvent ev;
		s_managerData->eventCallback(ev);
	}
}
