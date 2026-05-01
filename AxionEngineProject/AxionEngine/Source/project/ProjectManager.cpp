#include "axpch.h"
#include "ProjectManager.h"

#include "AxionEngine/Source/events/ApplicationEvent.h"
#include "AxionEngine/Source/events/RenderingEvent.h"
#include "AxionEngine/Source/project/Project.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/scripting/ScriptEngine.h"

namespace Axion {

	struct ProjectManagerData {
		bool isRuntime = false;
		Ref<Project> project = nullptr;
		std::filesystem::path projectPath;
		std::function<void(Event&)> eventCallback;

		// -- New project --
		bool newProjectRequest = false;
		ProjectSpecification newProjectSpecification;

		// -- Load project --
		bool loadProjectRequest = false;
		std::filesystem::path toLoadProjectPath;

		// -- Save project --
		bool saveProjectRequest = false;
		std::filesystem::path toSaveProjectPath;

		// -- Unload project --
		bool unloadProjectRequest = false;

		// -- Async Script Compilation --
		std::atomic<bool> isCompiling = false;
		std::atomic<bool> compileFinished = false;
		std::atomic<bool> compileSuccess = false;
		std::filesystem::path pendingAssemblyPath;
	};

	static ProjectManagerData* s_managerData;

	void ProjectManager::initialize(std::function<void(Event&)> eventCallback) {
		s_managerData = new ProjectManagerData();
		s_managerData->eventCallback = eventCallback;
	}

	void ProjectManager::shutdown() {
		delete s_managerData;
	}

	void ProjectManager::onEvent(Event& e) {
		EventDispatcher dispatcher(e);

		dispatcher.dispatch<RenderingFinishedEvent>([](RenderingFinishedEvent& e) {

			// -- Process Async Compilation Result --
			if (s_managerData->compileFinished) {
				s_managerData->compileFinished = false;

				if (s_managerData->compileSuccess) {
					ScriptEngine::loadAppAssembly(s_managerData->pendingAssemblyPath);
					AX_CORE_LOG_INFO("Assembly loaded successfully after async compilation.");
				}
				else {
					AX_CORE_LOG_ERROR("Skipped loading assembly due to compilation errors.");
				}
			}

			// -- Save Project --
			if (s_managerData->saveProjectRequest) {
				std::filesystem::path filepath = s_managerData->toSaveProjectPath;
				if (!filepath.empty() && std::filesystem::exists(filepath.parent_path())) {
					s_managerData->project->save(filepath);
					AX_CORE_LOG_INFO("Project Saved");
				}
				else {
					AX_CORE_LOG_ERROR("Unable to save Project!");
				}

				s_managerData->saveProjectRequest = false;
				s_managerData->toSaveProjectPath.clear();
			}

			// -- Load Project --
			if (s_managerData->loadProjectRequest) {
				std::filesystem::path filePath = s_managerData->toLoadProjectPath;
				if (!filePath.empty() && std::filesystem::exists(filePath)) {
					setProject(Project::load(filePath));
					s_managerData->projectPath = filePath;

					if (!s_managerData->project->getDefaultScene().empty()) {
						SceneManager::loadScene(s_managerData->project->getDefaultScene());
						AX_CORE_LOG_TRACE("Loaded Default Scene: {}", SceneManager::getScene()->getTitle());
					}
					else {
						SceneManager::newScene();
						AX_CORE_LOG_WARN("Unable loading Default Scene, loaded an emptry Scene!");
					}

					triggerScriptAssemblyLoad();

					AX_CORE_LOG_INFO("Project Loaded!");
				}
				else {
					AX_CORE_LOG_ERROR("Unable to load Project!");
				}

				s_managerData->loadProjectRequest = false;
				s_managerData->toLoadProjectPath.clear();
			}

			// -- New Project --
			if (s_managerData->newProjectRequest) {
				Ref<Project> newProject = Project::createNew(s_managerData->newProjectSpecification);
				if (newProject) {
					AX_CORE_LOG_INFO("New Project created");

					setProject(newProject);

					std::string safeName = s_managerData->newProjectSpecification.name;
					std::replace(safeName.begin(), safeName.end(), ' ', '_');
					s_managerData->projectPath = s_managerData->newProjectSpecification.location / safeName / (safeName + ".axproj");

					SceneManager::newScene();

					triggerScriptAssemblyLoad();

					AX_CORE_LOG_TRACE("New Project loaded");
				}
				else {
					AX_CORE_LOG_ERROR("Unable creating new Project");
				}

				s_managerData->newProjectRequest = false;
				s_managerData->newProjectSpecification = {};
			}

			// -- Unload Project --
			if (s_managerData->unloadProjectRequest) {
				setProject(nullptr);
				s_managerData->projectPath.clear();
				AX_CORE_LOG_INFO("Unloaded Project");

				s_managerData->unloadProjectRequest = false;
			}

			return false;
		});

	}

	void ProjectManager::newProject(const ProjectSpecification& spec) {
		// -- Auto Save active Project and request a new Project --
		if (hasProject()) {
			saveProject(s_managerData->projectPath);
		}

		s_managerData->newProjectSpecification = spec;
		s_managerData->newProjectRequest = true;
	}

	void ProjectManager::loadProject(const std::filesystem::path& filePath) {
		// -- Auto Save active Project and request loading --
		if (hasProject()) {
			saveProject(s_managerData->projectPath);
		}

		s_managerData->toLoadProjectPath = filePath;
		s_managerData->loadProjectRequest = true;
	}

	void ProjectManager::loadRuntimeProject(const std::filesystem::path& configFilePath) {
		s_managerData->isRuntime = true;

		// -- Load Runtime Project --
		Ref<Project> runtimeProject = Project::loadBinary(configFilePath);
		if (runtimeProject) {
			setProject(runtimeProject);
			s_managerData->projectPath = configFilePath;
			triggerScriptAssemblyLoad();
			AX_CORE_LOG_INFO("Runtime Project Loaded: {}", runtimeProject->getName());
		}
		else {
			AX_CORE_LOG_ERROR("Failed to bootstrap Runtime Project!");
		}
	}

	void ProjectManager::saveProject(const std::filesystem::path& filePath) {
		s_managerData->toSaveProjectPath = filePath;
		s_managerData->saveProjectRequest = true;
	}

	void ProjectManager::unloadProject() {
		// -- Auto Save active Project and request saving --
		if (hasProject()) {
			saveProject(s_managerData->projectPath);
		}

		s_managerData->unloadProjectRequest = true;
	}

	Ref<Project> ProjectManager::getProject() { return s_managerData->project; }

	bool ProjectManager::hasProject() { return s_managerData && s_managerData->project != nullptr; }

	const std::filesystem::path& ProjectManager::getProjectFilePath() { return s_managerData->projectPath; }

	void ProjectManager::setProject(const Ref<Project>& project) {
		s_managerData->project = project;
		AX_CORE_ASSERT(s_managerData->eventCallback, "Invalid event callback for project manager")
		ProjectChangedEvent ev;
		s_managerData->eventCallback(ev);
	}

	void ProjectManager::setRuntime() {
		s_managerData->isRuntime = true;
	}

	bool ProjectManager::isRuntime() {
		return s_managerData->isRuntime;
	}

	void ProjectManager::triggerScriptAssemblyLoad() {
		std::filesystem::path gameAssemblyPath;

		if (s_managerData->isRuntime) {
			gameAssemblyPath = "GameAssembly.dll";
			ScriptEngine::loadAppAssembly(gameAssemblyPath);
		}
		else if (s_managerData->project) {
			std::filesystem::path projectDir = s_managerData->projectPath.parent_path();
			gameAssemblyPath = projectDir / "Scripts" / "bin" / "Debug" / "net10.0" / "GameAssembly.dll";
			std::filesystem::path csprojPath = projectDir / "Scripts" / "GameAssembly.csproj";

			if (!std::filesystem::exists(csprojPath)) {
				AX_CORE_LOG_WARN("Legacy project detected. Upgrading project to support C# scripting...");
				Project::generateScriptProject(projectDir);
			}

			s_managerData->pendingAssemblyPath = gameAssemblyPath;
			s_managerData->isCompiling = true;
			s_managerData->compileFinished = false;

			AX_CORE_LOG_INFO("Starting Background Script Compilation...");

			std::thread([csprojPath]() {
				bool success = ScriptEngine::compileAppAssembly(csprojPath);

				s_managerData->compileSuccess = success;
				s_managerData->compileFinished = true;
				s_managerData->isCompiling = false;
			}).detach();
		}
	}

	bool ProjectManager::isCompilingScripts() {
		return s_managerData && s_managerData->isCompiling;
	}

}
