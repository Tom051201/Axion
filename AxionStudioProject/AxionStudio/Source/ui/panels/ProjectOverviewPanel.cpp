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
#include <Silica/include/SWrappedTextBlock.h>
#include <Silica/include/SScissorBox.h>
#include <Silica/include/SImage.h>
#include <Silica/include/SMenuAnchor.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

#include "AxionAssetPipeline/Source/core/AssetPackager.h"

#include "AxionStudio/Source/core/EditorSettings.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"
#include "AxionStudio/Source/ui/EditorTheme.h"

namespace {
	constexpr float VERSION_BOX_X = 50.0f;
}

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
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({ .borderThickness = Silica::GetTheme().Border_Thickness });
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
			m_uiRoot->setChild(SilicaHelpers::MakeEmptyState("No Project Loaded.\n\nPlease load or create a project from the top menu bar to view project settings."));
			return;
		}

		// -- Options Menu --
		auto optionsMenu = Silica::MakeWidget<Silica::SAlign>({
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = Silica::MakeWidget<Silica::SMenuAnchor>({
				.openOnHover = false,
				.openToRight = true,
				.anchorContent = Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
					.color = Silica::Color::transparent(),
					.hoverColor = Silica::Color(255, 255, 255, 20),
					.onClick = []() { return Silica::EventReply::unhandled(); },
					.child = Silica::MakeWidget<Silica::SImage>({
						.textureID = SilicaContext::getIcon("GearIcon"),
						.tint = Silica::GetTheme().Text_Main,
						.desiredSize = { EditorTheme::ICON_SIZE_SMALL, EditorTheme::ICON_SIZE_SMALL }
					})
				}),
				.menuContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
					.explicitSize = Silica::Vec2{ EditorTheme::OPTIONS_MENU_WIDTH, 0.0f },
					.borderThickness = Silica::GetTheme().Border_Thickness,
					.backgroundColor = Silica::GetTheme().Background_Popup,
					.child = Silica::MakeWidget<Silica::SVerticalBox>({
						.spacing = EditorTheme::SPACING_SMALL,
						.slots = {
							{ {0,0}, SilicaHelpers::MakeOptionsMenuItem("Open Project Directory", [this]() {
								PlatformUtils::openFolderInFileExplorer(m_project->getProjectPath());
							}) }
						}
					})
				})
			})
		});

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
			.padding = { EditorTheme::TOOLBAR_PADDING_X, 0.0f },
			.explicitSize = Silica::Vec2{ 0.0f, EditorTheme::TOOLBAR_HEIGHT },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::TOOLBAR_SPACING,
				.slots = {
					{ {0, 0}, optionsMenu },
					{ {0, 0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({
							.text = "Project:",
							.color = Silica::GetTheme().Text_Dim
						})
					})},
					{ {1, 0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = nameInput
					})}
				}
			})
			});

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = EditorTheme::SPACING_LARGE });

		// -- Game Version --
		Version projectVersion = m_project->getVersion();

		auto makeVersionBox = [this](int value, auto onCommit) {
			return Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2(VERSION_BOX_X, 0.0f),
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = std::to_string(value),
					.onTextCommitted = onCommit
				})
			});
		};

		auto versionRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::SPACING_MEDIUM,
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
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Version", versionRow, EditorTheme::PROPERTY_ROW_LABEL_WIDTH) });


		// -- App Icon --
		std::filesystem::path currentIcon = m_project->getAppIconPath();
		std::string iconDisplay = currentIcon.empty() ? "None" : currentIcon.filename().string();

		auto iconRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::SPACING_LARGE,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = iconDisplay })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
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
					.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
					.color = Silica::GetTheme().Accent_Danger,
					.onClick = [this]() {
						m_project->setAppIconPath("");
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
						rebuildUI();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X" })
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("App Icon (.ico)", iconRow, EditorTheme::PROPERTY_ROW_LABEL_WIDTH) });


		// -- Project And Assets Folders --
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Project File", Silica::MakeWidget<Silica::STextBlock>({
			.text = m_projectFileRelative.string(),
			.color = Silica::GetTheme().Text_Dim
		}), EditorTheme::PROPERTY_ROW_LABEL_WIDTH) });

		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Assets Path", Silica::MakeWidget<Silica::STextBlock>({
			.text = m_assetsRelative.string(),
			.color = Silica::GetTheme().Text_Dim
		}), EditorTheme::PROPERTY_ROW_LABEL_WIDTH) });


		// -- Default Scene --
		std::filesystem::path currentDefault = m_project->getDefaultScene();
		std::string sceneDisplayStr = "None (Drag .axscene here)";
		if (!currentDefault.empty()) {
			std::filesystem::path defaultScenePath = AssetManager::getRelativeToAssets(currentDefault);
			sceneDisplayStr = defaultScenePath.filename().string();
		}

		auto sceneDropZoneInner = Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = sceneDisplayStr })
		});

		auto sceneDropZone = SilicaHelpers::MakeAssetDropZone(".axscene", [this](const std::filesystem::path& droppedPath) {
			std::filesystem::path absPath = AssetManager::getAbsolute(droppedPath);
			m_project->setDefaultScene(absPath);
			ProjectManager::saveProject(ProjectManager::getProjectFilePath());
			rebuildUI();
		}, sceneDropZoneInner, { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y });

		auto sceneRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::SPACING_MEDIUM,
			.slots = {
				{ {1,0}, sceneDropZone },
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
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
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Default Scene", sceneRow, EditorTheme::PROPERTY_ROW_LABEL_WIDTH) });


		// -- Show In Explorer --
		auto explorerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::SPACING_MEDIUM,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
					.onClick = [this]() {
						PlatformUtils::openFolderInFileExplorer(m_project->getProjectPath());
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Project Folder" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
					.onClick = [this]() {
						PlatformUtils::openFolderInFileExplorer(m_project->getAssetsPath());
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Assets Folder" })
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Show in Explorer", explorerRow, EditorTheme::PROPERTY_ROW_LABEL_WIDTH) });


		// -- Options --
		auto optionsRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::SPACING_MEDIUM,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
					.color = Silica::GetTheme().Accent_Primary,
					.onClick = [this]() {
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Save Project" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
					.onClick = [this]() {
						EditorSettings::startupProjectPath = ProjectManager::getProjectFilePath();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Set As Startup" })
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
					.color = Silica::GetTheme().Accent_Warning,
					.onClick = [this]() {
						if (m_openExportModalCallback) m_openExportModalCallback();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Export Game" })
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Options", optionsRow, EditorTheme::PROPERTY_ROW_LABEL_WIDTH) });


		// -- Final Layout Assembly --
		auto paddedContent = Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::PADDING_LARGE, EditorTheme::PADDING_LARGE },
			.child = contentBox
		});

		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({ .child = paddedContent });

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
