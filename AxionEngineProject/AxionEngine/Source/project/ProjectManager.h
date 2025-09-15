#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/project/Project.h"

#include <functional>

namespace Axion {

	class ProjectManager {
	public:

		static void initialize(std::function<void(Event&)> eventCallback);
		static void release();
		static void onEvent(Event& e);

		static void newProject(const ProjectSpecification& spec);
		static void loadProject(const std::string& filePath);
		static void saveProject(const std::string& filePath);
		static Ref<Project> getProject();
		static bool hasProject();

	private:

		static void setProject(const Ref<Project>& project);

	};

}
