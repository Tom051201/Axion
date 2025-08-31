#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/project/Project.h"

namespace Axion {

	class ProjectPanel {
	public:

		ProjectPanel();
		~ProjectPanel();

		void setup();
		void shutdown();

		void setProject(const Ref<Project>& project);

		void onGuiRender();

	private:

		Ref<Project> m_project;

	};

}
