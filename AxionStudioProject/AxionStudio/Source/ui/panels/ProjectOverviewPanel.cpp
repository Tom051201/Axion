#include "studiopch.h"
#include "ProjectOverviewPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SAlign.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

#include "AxionAssetPipeline/Source/core/AssetPackager.h"

#include "AxionStudio/Source/core/EditorConfig.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	void ProjectPanel::setProject(const Shared<Project>& project) {
		m_project = project;
		if (ProjectManager::hasProject()) {
			m_rootDirectory = m_project->getProjectPath().parent_path();
			m_projectFileRelative = std::filesystem::relative(project->getProjectPath(), m_rootDirectory);
			m_assetsRelative = std::filesystem::relative(project->getAssetsPath(), m_rootDirectory);
		}
		rebuildUI();
	}

	Silica::WidgetPtr ProjectPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({.borderThickness = Silica::GetTheme().Border_Thickness });
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void ProjectPanel::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void ProjectPanel::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		// -- No project loaded --
		if (!ProjectManager::hasProject()) {
			m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "No Project Loaded.\nPlease load or create a project first."
				})
			}));
			return;
		}

		// -- Name Input --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::GetTheme().Background_Input,
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_project->getName(),
				.onTextCommitted = [this](const std::string& newText) {
					m_project->setName(newText);
					ProjectManager::saveProject(ProjectManager::getProjectFilePath());
				}
			})
		});

		auto topBarBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { 10.0f, 10.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({
							.text = "Project:",
							.color = Silica::GetTheme().Text_Dim
						})
					})},
					{ {1, 0}, nameInput }
				}
			})
		});

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 15.0f });

		// -- Helper Function --
		auto MakePropertyRow = [&](const std::string& label, Silica::WidgetPtr valueWidget) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(140.0f, 0.0f),
						.backgroundColor = Silica::Color::transparent(),
						.child = Silica::MakeWidget<Silica::SAlign>({
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = label })
						})
					})},
					{ {1, 0}, valueWidget }
				}
			});
		};

		// -- Game Version --
		Version projectVersion = m_project->getVersion();

		auto makeVersionBox = [this](int value, auto onCommit) {
			return Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2(50.0f, 0.0f),
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = std::to_string(value),
					.onTextCommitted = onCommit
				})
			});
		};

		auto versionRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 5.0f,
			.slots = {
				{ {0,0}, makeVersionBox(projectVersion.major, [this, projectVersion](const std::string& val) mutable {
					try { projectVersion.major = std::max(0, std::stoi(val)); m_project->setVersion(projectVersion); }
					catch (...) {}
				})},
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "." }) },
				{ {0,0}, makeVersionBox(projectVersion.minor, [this, projectVersion](const std::string& val) mutable {
					try { projectVersion.minor = std::max(0, std::stoi(val)); m_project->setVersion(projectVersion); }
					catch (...) {}
				})},
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "." }) },
				{ {0,0}, makeVersionBox(projectVersion.patch, [this, projectVersion](const std::string& val) mutable {
					try { projectVersion.patch = std::max(0, std::stoi(val)); m_project->setVersion(projectVersion); }
					catch (...) {}
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Version", versionRow) });


		// -- App Icon --
		std::filesystem::path currentIcon = m_project->getAppIconPath();
		std::string iconDisplay = currentIcon.empty() ? "None" : currentIcon.filename().string();

		auto iconRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({ .text = iconDisplay })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [this]() {
						std::filesystem::path path = FileDialogs::openFile({ {"Windows Icon", "*.ico"} });
						if (!path.empty()) {
							m_project->setAppIconPath(path);
							ProjectManager::saveProject(ProjectManager::getProjectFilePath());
							rebuildUI();
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.color = Silica::GetTheme().Accent_Danger,
					.onClick = [this]() {
						m_project->setAppIconPath("");
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
						rebuildUI();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "X" })
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("App Icon (.ico)", iconRow) });


		// -- Project And Assets Folders --
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Project File", Silica::MakeWidget<Silica::STextBlock>({
			.text = m_projectFileRelative.string(),
			.color = Silica::GetTheme().Text_Dim
		})) });
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Assets Path", Silica::MakeWidget<Silica::STextBlock>({
			.text = m_assetsRelative.string(),
			.color = Silica::GetTheme().Text_Dim
		})) });


		// -- Default Scene --
		std::filesystem::path currentDefault = m_project->getDefaultScene();
		std::string sceneDisplayStr = "None (Drag .axscene here)";
		if (!currentDefault.empty()) {
			std::filesystem::path defaultScenePath = AssetManager::getRelativeToAssets(currentDefault);
			sceneDisplayStr = defaultScenePath.filename().string();
		}

		auto sceneRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.padding = { 8.0f, 4.0f },

					// -- Catch Native Drag / Drop --
					.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
						if (payload.type == "AssetPath") {
							std::filesystem::path droppedPath = std::any_cast<std::filesystem::path>(payload.data);

							if (droppedPath.extension() == ".axscene") {
								std::filesystem::path absPath = AssetManager::getAbsolute(droppedPath);
								m_project->setDefaultScene(absPath);
								ProjectManager::saveProject(ProjectManager::getProjectFilePath());
								rebuildUI();
								return Silica::EventReply::handled();
							}
						}
						return Silica::EventReply::unhandled();
					},

					.child = Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = sceneDisplayStr })
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [this]() {
						std::filesystem::path currentScenePath = SceneManager::getScenePath();
						if (!currentScenePath.empty()) {
							std::filesystem::path absPath = AssetManager::getAbsolute(currentScenePath);
							m_project->setDefaultScene(absPath);
							ProjectManager::saveProject(ProjectManager::getProjectFilePath());
							rebuildUI();
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Set Current" })
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Default Scene", sceneRow) });


		// -- Show In Explorer --
		auto explorerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [this]() {
						PlatformUtils::openFolderInFileExplorer(m_project->getProjectPath());
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Project Folder" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [this]() {
						PlatformUtils::openFolderInFileExplorer(m_project->getAssetsPath());
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Assets Folder" })
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Show in Explorer", explorerRow) });


		// -- Options --
		auto optionsRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.color = Silica::GetTheme().Accent_Primary,
					.onClick = [this]() {
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Save Project" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [this]() {
						EditorConfig::startupProjectPath = ProjectManager::getProjectFilePath();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Set As Startup" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.color = Silica::GetTheme().Accent_Warning,
					.onClick = [this]() {
						if (m_openExportModalCallback) m_openExportModalCallback();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Export Game" })
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Options", optionsRow) });


		// -- Final Layout Assembly --
		auto paddedContent = Silica::MakeWidget<Silica::SBox>({
			.padding = { 10.0f, 10.0f },
			.child = contentBox
		});

		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({
			.child = paddedContent
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = topBarBox,
			.contentArea = scrollBox
		}));
	}

	void ProjectPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(ProjectPanel::onProjectChanged));
	}

	EventReply ProjectPanel::onProjectChanged(ProjectChangedEvent& e) {
		setProject(ProjectManager::getProject());
		return EventReply::unhandled();
	}

}
