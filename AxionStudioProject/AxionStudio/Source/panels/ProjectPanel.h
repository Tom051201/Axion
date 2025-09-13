#pragma once

#include "AxionStudio/Source/core/Panel.h"

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/project/Project.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

namespace Axion {

	class ProjectPanel : public Panel {
	public:

		ProjectPanel(const std::string& name);
		~ProjectPanel() override;

		void setup() override;
		void shutdown() override;
		void onEvent(Event& e) override;
		void onGuiRender() override;

		void setProject(const Ref<Project>& project);

	private:

		Ref<Project> m_project;

		std::filesystem::path m_rootDirectory;
		std::string m_projectFileDisplay;
		std::string m_assetsDisplay;
		std::string m_scenesDisplay;

		bool onProjectChanged(ProjectChangedEvent& e);

	};

}
