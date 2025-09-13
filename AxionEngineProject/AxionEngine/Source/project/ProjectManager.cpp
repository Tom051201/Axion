#include "axpch.h"
#include "ProjectManager.h"

#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/project/Project.h"

namespace Axion {

	struct ProjectManagerData {
		Ref<Project> activeProject;
		std::function<void(Event&)> eventCallback;
	};

	static ProjectManagerData* s_managerData;

	void ProjectManager::initialize(std::function<void(Event&)> eventCallback) {
		s_managerData = new ProjectManagerData();
		s_managerData->eventCallback = eventCallback;
	}

	void ProjectManager::release() {
		delete s_managerData;
	}

	void ProjectManager::setActiveProject(const Ref<Project>& project) {
		s_managerData->activeProject = project;

		if (s_managerData->eventCallback) {
			ProjectChangedEvent ev;
			s_managerData->eventCallback(ev);
		}
	}

	Ref<Project> ProjectManager::getActiveProject() { return s_managerData->activeProject; }

	bool ProjectManager::hasActiveProject() { return s_managerData && s_managerData->activeProject != nullptr; }

}
