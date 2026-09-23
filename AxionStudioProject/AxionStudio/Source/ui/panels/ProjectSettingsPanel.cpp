#include "studiopch.h"
#include "ProjectSettingsPanel.h"

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
#include <Silica/include/SInputFieldInt.h>
#include <Silica/include/SSpacer.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/physics/PhysicsLayerManager.h"

#include "AxionAssetPipeline/Source/core/AssetPackager.h"

#include "AxionStudio/Source/core/EditorSettings.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"
#include "AxionStudio/Source/ui/EditorTheme.h"

namespace {
	constexpr float VERSION_BOX_X = 30.0f;
	constexpr float DROP_ZONE_PADDING = 4.0f;
}

namespace Axion {

	void ProjectSettingsPanel::setProject(const Shared<Project>& project) {
		m_project = project;
		if (ProjectManager::hasProject()) {
			m_rootDirectory = m_project->getProjectPath().parent_path();
			m_projectFileRelative = std::filesystem::relative(project->getProjectPath(), m_rootDirectory);
			m_assetsRelative = std::filesystem::relative(project->getAssetsPath(), m_rootDirectory);
		}
		rebuildUI();
	}

	Silica::WidgetPtr ProjectSettingsPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({.hasBorder = true });
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void ProjectSettingsPanel::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void ProjectSettingsPanel::rebuildUI_Internal() {
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
					.hasBorder = true,
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

		auto topBarBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::TOOLBAR_PADDING_X, 0.0f },
			.explicitSize = Silica::Vec2{ 0.0f, EditorTheme::TOOLBAR_HEIGHT },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::TOOLBAR_SPACING,
				.slots = {
					{ {0, 0}, optionsMenu },
					{ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { EditorTheme::SPACING_LARGE, 0.0f } }) },
					{ {0, 0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Project Settings" })
					}) }
				}
			})
		});

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE });

		// ----- GENERAL -----
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakeHeader("General") });

		auto generalContent = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE });

		auto nameInput = Silica::MakeWidget<Silica::SEditableText>({
			.initialText = m_project->getName(),
			.onTextCommitted = [this](const std::string& newText) {
				m_project->setName(newText);
				ProjectManager::saveProject(ProjectManager::getProjectFilePath());
			}
		});
		generalContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Project Name:", nameInput) });

		generalContent->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("Project File:", m_projectFileRelative.string()) });
		generalContent->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("Assets Directory:", m_assetsRelative.string()) });

		// -- Project version --
		Version projectVersion = m_project->getVersion();
		auto makeVersionBox = [this](int value, std::function<void(int)> onValueChanged) {
			return Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2(VERSION_BOX_X, 0.0f),
				.child = Silica::MakeWidget<Silica::SInputFieldInt>({
					.initialValue = value,
					.onValueChanged = onValueChanged
				})
			});
		};

		auto versionRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::SPACING_MEDIUM,
			.slots = {
				{ {0,0}, makeVersionBox(projectVersion.major, [this, projectVersion](int val) mutable {
					projectVersion.major = val;
					m_project->setVersion(projectVersion);
				})},
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "." }) },
				{ {0,0}, makeVersionBox(projectVersion.minor, [this, projectVersion](int val) mutable {
					projectVersion.minor = val;
					m_project->setVersion(projectVersion);
				})},
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "." }) },
				{ {0,0}, makeVersionBox(projectVersion.patch, [this, projectVersion](int val) mutable {
					projectVersion.patch = val;
					m_project->setVersion(projectVersion);
				})}
			}
		});
		generalContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Version:", versionRow) });

		// -- Show In Explorer --
		auto projDirBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.onClick = [this]() {
				PlatformUtils::openFolderInFileExplorer(m_project->getProjectPath());
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Project Folder" })
		});

		auto assetsDirBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.onClick = [this]() {
				PlatformUtils::openFolderInFileExplorer(m_project->getAssetsPath());
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Assets Folder" })
		});

		auto explorerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::SPACING_MEDIUM,
			.slots = {
				{ {0,0}, projDirBtn},
				{ {0,0}, assetsDirBtn }
			}
		});
		generalContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Show in Explorer:", explorerRow) });

		// -- Options --
		auto saveBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.color = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				ProjectManager::saveProject(ProjectManager::getProjectFilePath());
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Save Project" })
		});

		auto startupBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.onClick = [this]() {
				EditorSettings::startupProjectPath = ProjectManager::getProjectFilePath();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Set As Startup" })
		});

		auto exportBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.color = Silica::GetTheme().Accent_Warning,
			.onClick = [this]() {
				if (m_openExportModalCallback) m_openExportModalCallback();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Export Project" })
		});

		auto optionsRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::SPACING_MEDIUM,
			.slots = {
				{ {0,0}, saveBtn },
				{ {0,0}, startupBtn },
				{ {0,0}, exportBtn }
			}
		});
		generalContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Options:", optionsRow) });

		auto generalDummyZone = SilicaHelpers::MakeAssetDropZone("", [](const std::filesystem::path&) {}, generalContent, { 4.0f, 4.0f });
		contentBox->addSlot({ {0, 0}, generalDummyZone });

		contentBox->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { 0.0f, EditorTheme::SPACING_LARGE }}) });

		// ----- APP ICON -----
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakeHeader("App Icon") });

		auto appIconContent = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE });

		if (!m_project->getAppIconPath().empty()) {
			std::filesystem::path currentIcon = m_project->getAppIconPath();

			appIconContent->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("File Name:", currentIcon.filename().string()) });
			appIconContent->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("File Path:", currentIcon.generic_string()) });

			auto changeBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.onClick = [this]() {
					std::filesystem::path path = FileDialogs::openFile({ {"Windows Icon", "*.ico"} }, ProjectManager::getProject()->getAssetsPath());

					if (!path.empty()) {
						m_project->setAppIconPath(path);
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
						rebuildUI();
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Change Icon"})
			});

			auto removeBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.color = Silica::GetTheme().Accent_Danger,
				.onClick = [this]() {
					m_project->setAppIconPath("");
					ProjectManager::saveProject(ProjectManager::getProjectFilePath());
					rebuildUI();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Remove"})
			});

			auto locateBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.onClick = [this, currentIcon]() {
					// TODO: fix this not opening the correct folder
					//PlatformUtils::openFolderInFileExplorer(currentIcon.parent_path());
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Locate"})
			});

			auto options = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::SPACING_MEDIUM,
				.slots = {
					{ {0,0}, changeBtn },
					{ {0,0}, removeBtn },
					{ {0,0}, locateBtn },
				}
			});

			appIconContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Options:", options) });

		}
		else {
			auto iconBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.onClick = [this]() {

					std::filesystem::path path = FileDialogs::openFile({ {"Windows Icon", "*.ico"} }, ProjectManager::getProject()->getAssetsPath());
					if (!path.empty()) {
						m_project->setAppIconPath(path);
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
						rebuildUI();
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse" })
			});

			appIconContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Options:", iconBtn) });
		}

		auto iconDropZone = SilicaHelpers::MakeAssetDropZone(".ico", [this](const std::filesystem::path& droppedPath) {
			std::filesystem::path absPath = AssetManager::getAbsolute(droppedPath);
			m_project->setAppIconPath(absPath);
			ProjectManager::saveProject(ProjectManager::getProjectFilePath());
			rebuildUI();
		}, appIconContent, { 4.0f, 4.0f });

		contentBox->addSlot({ {0, 0}, iconDropZone });
		contentBox->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { 0.0f, EditorTheme::SPACING_LARGE }}) });



		// ----- DEFAULT SCENE -----
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakeHeader("Default Scene") });

		std::filesystem::path currentDefault = m_project->getDefaultScene();

		auto defaultSceneContent = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE});

		if (!currentDefault.empty()) {
			defaultSceneContent->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("File Name:", currentDefault.filename().string()) });
			defaultSceneContent->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("File Path:", currentDefault.generic_string()) });

			auto changeBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.onClick = [this]() {
					std::filesystem::path sceDir = ProjectManager::getProject()->getAssetsPath() / "Scenes";
					std::filesystem::path absolutePath = std::filesystem::exists(sceDir) ?
						FileDialogs::openFile({ {"Axion Scene", "*.axscene"} }, sceDir) :
						FileDialogs::openFile({ {"Axion Scene", "*.axscene"} }, ProjectManager::getProject()->getAssetsPath());

					if (!absolutePath.empty()) {
						UUID assetUUID = AssetManager::getAssetUUID(absolutePath);
						if (assetUUID.isValid()) {
							std::string genericStr = absolutePath.generic_string();
							m_project->setDefaultScene(std::filesystem::path(genericStr));
							ProjectManager::saveProject(ProjectManager::getProjectFilePath());
							rebuildUI();
						}
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Change Scene"})
			});

			auto removeBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.color = Silica::GetTheme().Accent_Danger,
				.onClick = [this]() {
					m_project->setDefaultScene("");
					ProjectManager::saveProject(ProjectManager::getProjectFilePath());
					rebuildUI();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Remove"})
			});

			auto locateBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.onClick = [this, currentDefault]() {
					// TODO: fix this not opening the correct folder
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Locate"})
			});

			auto options = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::SPACING_MEDIUM,
				.slots = {
					{ {0,0}, changeBtn },
					{ {0,0}, removeBtn },
					{ {0,0}, locateBtn },
				}
			});
			defaultSceneContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Options:", options) });
		}
		else {
			auto sceneBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.onClick = [this]() {
					std::filesystem::path sceDir = ProjectManager::getProject()->getAssetsPath() / "Scenes";
					std::filesystem::path absolutePath = std::filesystem::exists(sceDir) ?
						FileDialogs::openFile({ {"Axion Scene", "*.axscene"} }, sceDir) :
						FileDialogs::openFile({ {"Axion Scene", "*.axscene"} }, ProjectManager::getProject()->getAssetsPath());

					if (!absolutePath.empty()) {
						UUID assetUUID = AssetManager::getAssetUUID(absolutePath);
						if (assetUUID.isValid()) {
							std::string genericStr = absolutePath.generic_string();
							m_project->setDefaultScene(std::filesystem::path(genericStr));
							ProjectManager::saveProject(ProjectManager::getProjectFilePath());
							rebuildUI();
						}
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..."})
			});

			defaultSceneContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Options:", sceneBtn) });
		}

		auto sceneDropZone = SilicaHelpers::MakeAssetDropZone(".axscene", [this](const std::filesystem::path& droppedPath) {
			std::filesystem::path absPath = AssetManager::getAbsolute(droppedPath);
			m_project->setDefaultScene(absPath);
			ProjectManager::saveProject(ProjectManager::getProjectFilePath());
			rebuildUI();
		}, defaultSceneContent, { 4.0f, 4.0f });

		contentBox->addSlot({ {0, 0}, sceneDropZone });
		contentBox->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { 0.0f, EditorTheme::SPACING_LARGE }}) });


		// ----- PHYSICS LAYERS -----
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakeHeader("Physics Layers") }); // TODO: make this with a + button up to 31 times and add an event so that the properties panel rebuilds to show the new names and new things

		auto physicsLayersContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = EditorTheme::SPACING_SMALL });

		for (uint32_t i = 0; i < 32; ++i) {
			std::string label = "Layer " + std::to_string(i);

			if (i == 0) {
				auto defaultText = Silica::MakeWidget<Silica::STextBlock>({
					.text = "Default (Read-Only)",
					.color = Silica::GetTheme().Text_Dim
				});
				physicsLayersContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow(label, defaultText) });
			}
			else {
				auto nameInput = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = PhysicsLayerManager::getLayerName(i),
					.hintText = "...",
					.onTextCommitted = [this, i](const std::string& newText) {
						PhysicsLayerManager::setLayerName(i, newText);
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
					}
				});
				physicsLayersContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow(label, nameInput) });
			}
		}

		auto physicsDropZone = SilicaHelpers::MakeAssetDropZone("", [](const std::filesystem::path&) {}, physicsLayersContent, { DROP_ZONE_PADDING, DROP_ZONE_PADDING });

		contentBox->addSlot({ {0, 0}, physicsDropZone });
		contentBox->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { 0.0f, EditorTheme::SPACING_LARGE }}) });


		// -- Final Layout Assembly --
		auto paddedContent = Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::PADDING_LARGE, EditorTheme::PADDING_LARGE },
			.child = contentBox
		});

		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({.child = paddedContent });

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = topBarBox,
			.contentArea = scrollBox
		}));
	}

	void ProjectSettingsPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(ProjectSettingsPanel::onProjectChanged));
	}

	EventReply ProjectSettingsPanel::onProjectChanged(ProjectChangedEvent& e) {
		setProject(ProjectManager::getProject());
		return EventReply::unhandled();
	}

}
