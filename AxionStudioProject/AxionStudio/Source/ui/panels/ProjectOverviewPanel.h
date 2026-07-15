#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/project/Project.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

#include "AxionStudio/Vendor/Silica/include/SBox.h"


namespace Axion {

	class ProjectPanel {
	public:

		ProjectPanel() = default;
		~ProjectPanel() = default;

		void onEvent(Event& e);

		Silica::WidgetPtr getWidget();
		void setProject(const Ref<Project>& project);

		void setOpenExportModalCallback(const std::function<void()>& callback) { m_openExportModalCallback = callback; }

	private:

		Ref<Project> m_project;
		std::filesystem::path m_rootDirectory;
		std::filesystem::path m_projectFileRelative;
		std::filesystem::path m_assetsRelative;
		std::function<void()> m_openExportModalCallback;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

		void rebuildUI();
		void rebuildUI_Internal();

		bool onProjectChanged(ProjectChangedEvent& e);

	};

}
