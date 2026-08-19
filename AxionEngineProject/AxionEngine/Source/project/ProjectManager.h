#pragma once

#include <functional>
#include <filesystem>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/project/Project.h"

namespace Axion {

	class ProjectManager {
	public:

		static void initialize(std::function<void(Event&)> eventCallback);
		static void shutdown();
		static void onEvent(Event& e);

		static void newProject(const ProjectSpecification& spec);
		static void loadProject(const std::filesystem::path& filePath);
		static void loadRuntimeProject(const std::filesystem::path& configFilePath = "GameConfig.axbin"); // TODO: create an config folder for those
		static void saveProject(const std::filesystem::path& filePath);
		static void unloadProject();

		static Shared<Project> getProject();
		static bool hasProject();
		static const std::filesystem::path& getProjectFilePath();

		static void setRuntime();
		static bool isRuntime();

		static void triggerScriptAssemblyLoad();
		static bool isCompilingScripts();

	private:

		static void setProject(const Shared<Project>& project);

	};

}
