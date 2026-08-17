#pragma once

#include <functional>
#include <filesystem>
#include <memory>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/project/Project.h"
#include "AxionEngine/Source/events/ApplicationEvent.h"

namespace Silica {
	class SBox;
}

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
