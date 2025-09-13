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

		static void setActiveProject(const Ref<Project>& project);
		static Ref<Project> getActiveProject();
		static bool hasActiveProject();

	};

}
