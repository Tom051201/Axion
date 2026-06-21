#include "ProjectOverviewPanel.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

#include "AxionStudio/Source/core/EditorConfig.h"
#include "AxionAssetPipeline/Source/core/AssetPackager.h"
#include "AxionStudio/Source/ui/panels/ContentBrowserPanel.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	void ProjectPanel::setProject(const Ref<Project>& project) {
		m_project = project;
		if (ProjectManager::hasProject()) {
			m_rootDirectory = m_project->getProjectPath().parent_path();
			m_projectFileRelative = std::filesystem::relative(project->getProjectPath(), m_rootDirectory);
			m_assetsRelative = std::filesystem::relative(project->getAssetsPath(), m_rootDirectory);
		}
		rebuildUI();
	}

	Silica::WidgetPtr ProjectPanel::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(25, 25, 25, 255)
			});
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
					.text = "No Project Loaded.\nPlease load or create a project first.",
					.font = m_font
				})
			}));
			return;
		}

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
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = label, .font = m_font})
						})
					})},
					{ {1, 0}, valueWidget }
				}
			});
		};

		// -- Project Name --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_project->getName(),
				.font = m_font,
				.onTextCommitted = [this](const std::string& newText) {
					m_project->setName(newText);
					ProjectManager::saveProject(ProjectManager::getProjectFilePath());
				}
			})
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Project Name", nameInput) });


		// -- Game Version --
		Version projectVersion = m_project->getVersion();

		auto makeVersionBox = [this](int value, auto onCommit) {
			return Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2(50.0f, 0.0f),
				.backgroundColor = Silica::Color(35, 35, 35, 255),
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = std::to_string(value),
					.font = m_font,
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
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = ".", .font = m_font}) },
				{ {0,0}, makeVersionBox(projectVersion.minor, [this, projectVersion](const std::string& val) mutable {
					try { projectVersion.minor = std::max(0, std::stoi(val)); m_project->setVersion(projectVersion); }
					catch (...) {}
				})},
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = ".", .font = m_font}) },
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
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = iconDisplay, .font = m_font})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f }, .color = Silica::Color(50, 50, 50, 255),
					.onClick = [this]() {
						std::filesystem::path path = FileDialogs::openFile({ {"Windows Icon", "*.ico"} });
						if (!path.empty()) {
							m_project->setAppIconPath(path);
							ProjectManager::saveProject(ProjectManager::getProjectFilePath());
							rebuildUI();
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse", .font = m_font})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f }, .color = Silica::Color(150, 50, 50, 255),
					.onClick = [this]() {
						m_project->setAppIconPath("");
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
						rebuildUI();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("App Icon (.ico)", iconRow) });


		// -- Project And Assets Folders --
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Project File", Silica::MakeWidget<Silica::STextBlock>({.text = m_projectFileRelative.string(), .color = Silica::Color(150, 150, 150, 255), .font = m_font})) });
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Assets Path", Silica::MakeWidget<Silica::STextBlock>({.text = m_assetsRelative.string(), .color = Silica::Color(150, 150, 150, 255), .font = m_font})) });


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
					.backgroundColor = Silica::Color(40, 40, 40, 255),
					.onDrop = [this]() {
						// -- Catch Drag / Drop --
						if (!s_draggedAssetPath.empty() && s_draggedAssetPath.extension() == ".axscene") {
							std::filesystem::path absPath = AssetManager::getAbsolute(s_draggedAssetPath);
							m_project->setDefaultScene(absPath);
							ProjectManager::saveProject(ProjectManager::getProjectFilePath());
							s_draggedAssetPath.clear();
							rebuildUI();
							return Silica::EventReply::handled();
						}
						return Silica::EventReply::unhandled();
					},
					.child = Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = sceneDisplayStr, .font = m_font})
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f }, .color = Silica::Color(50, 50, 50, 255),
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
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Set Current", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Default Scene", sceneRow) });


		// -- Show In Explorer --
		auto explorerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f }, .color = Silica::Color(50, 50, 50, 255),
					.onClick = [this]() { PlatformUtils::openFolderInFileExplorer(m_project->getProjectPath()); return Silica::EventReply::handled(); },
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Project Folder", .font = m_font})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f }, .color = Silica::Color(50, 50, 50, 255),
					.onClick = [this]() { PlatformUtils::openFolderInFileExplorer(m_project->getAssetsPath()); return Silica::EventReply::handled(); },
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Assets Folder", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Show in Explorer", explorerRow) });


		// -- Options --
		auto optionsRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f }, .color = Silica::Color(40, 100, 150, 255),
					.onClick = [this]() {
						ProjectManager::saveProject(ProjectManager::getProjectFilePath());
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Save Project", .font = m_font})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f }, .color = Silica::Color(50, 50, 50, 255),
					.onClick = [this]() {
						EditorConfig::startupProjectPath = ProjectManager::getProjectFilePath();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Set As Startup", .font = m_font})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f }, .color = Silica::Color(150, 100, 40, 255),
					.onClick = [this]() {
						if (m_openExportModalCallback) m_openExportModalCallback();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Export Game", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Options", optionsRow) });


		// -- Final Layout Assembly --
		auto paddedContent = Silica::MakeWidget<Silica::SBox>({
			.padding = { 10.0f, 10.0f },
			.child = contentBox
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SScrollBox>({
			.child = paddedContent
		}));
	}

	void ProjectPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(ProjectPanel::onProjectChanged));
	}

	bool ProjectPanel::onProjectChanged(ProjectChangedEvent& e) {
		setProject(ProjectManager::getProject());
		return false;
	}

}
