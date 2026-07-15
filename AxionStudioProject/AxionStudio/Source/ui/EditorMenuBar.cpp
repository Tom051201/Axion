#include "EditorMenuBar.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/Theme.h"

#include "AxionStudio/Source/ui/modals/CreateProjectModal.h"
#include "AxionStudio/Source/ui/modals/ExportProjectModal.h"
#include "AxionStudio/Source/ui/modals/SystemInfoModal.h"
#include "AxionStudio/Source/ui/modals/SettingsModal.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorModalManager.h"

namespace Axion {

	Silica::Color menuBarBg = Silica::GetTheme().Background_Panel;
	Silica::Color dropDownBg = Silica::GetTheme().Background_Panel;
	constexpr float dropdownSpacing = 0.0f;
	constexpr Silica::Vec2 dropDownPadding = { 0.0f, 0.0f };
	constexpr Silica::Vec2 menuButtonPadding = { 0.0f, 2.0f };
	constexpr Silica::Vec2 appTitlePadding = { 15.0f, 6.0f };


	// ----- Helper Functions -----
	Silica::WidgetPtr MakeMenuItem(const std::string& text, std::function<Silica::EventReply()> onClick) {
		return Silica::MakeWidget<Silica::SButton>({
			.padding = { 12.0f, 4.0f },
			.color = dropDownBg,
			.hoverColor = Silica::GetTheme().Surface_Secondary,
			.onClick = onClick,
			.child = Silica::MakeWidget<Silica::STextBlock>({ .text = text })
		});
	}



	Silica::WidgetPtr EditorMenuBar::construct(std::shared_ptr<Silica::SDockSpace> dockspace) {
		// ----- FILE MENU -----
		auto fileMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.anchorContent = MakeMenuItem("File", []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						// -- NEW SCENE --
						{ dropDownPadding, MakeMenuItem("New Scene", []() { 
							EditorActionQueue::push([]() {
								SceneManager::newScene(); 
							});
							return Silica::EventReply::handled(); 
						}) },

						// -- LOAD SCENE --
						{ dropDownPadding, MakeMenuItem("Load Scene", []() { 
							std::filesystem::path initialDir = ProjectManager::hasProject() ? ProjectManager::getProject()->getAssetsPath() : "";
							std::filesystem::path filepath = FileDialogs::openFile({ {"Axion Scene", "*.axscene"} }, initialDir);
							
							if (!filepath.empty()) {
								EditorActionQueue::push([filepath]() {
									SceneManager::loadScene(filepath);
								});
							}
							return Silica::EventReply::handled(); 
						}) },

						// -- SAVE SCENE --
						{ dropDownPadding, MakeMenuItem("Save Scene", []() { 
							if (SceneManager::isNewScene() || SceneManager::getScenePath().empty()) {
								std::filesystem::path initialDir = ProjectManager::hasProject() ? ProjectManager::getProject()->getAssetsPath() : "";
								std::filesystem::path filepath = FileDialogs::saveFile({ {"Axion Scene", "*.axscene"} }, initialDir);
								
								if (!filepath.empty()) {
									if (filepath.extension() != ".axscene") filepath += ".axscene";
									EditorActionQueue::push([filepath]() {
										SceneManager::saveScene(filepath);
									});
								}
							} else {
								EditorActionQueue::push([]() {
									SceneManager::saveScene(SceneManager::getScenePath());
								});
							}
							return Silica::EventReply::handled(); 
						}) },

						// -- SAVE SCENE AS --
						{ dropDownPadding, MakeMenuItem("Save Scene As...", []() { 
							std::filesystem::path initialDir = ProjectManager::hasProject() ? ProjectManager::getProject()->getAssetsPath() : "";
							std::filesystem::path filepath = FileDialogs::saveFile({ {"Axion Scene", "*.axscene"} }, initialDir);
							
							if (!filepath.empty()) {
								if (filepath.extension() != ".axscene") filepath += ".axscene";
								EditorActionQueue::push([filepath]() {
									SceneManager::saveScene(filepath);
								});
							}
							return Silica::EventReply::handled(); 
						}) },

						// -- EXIT --
						{ dropDownPadding, MakeMenuItem("Exit", []() { 
							//Application::get().close();
							// TODO: fix the crash
							return Silica::EventReply::handled(); 
						}) }
					}
				})
			}),
		});

		// ----- EDIT MENU -----
		auto editMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.anchorContent = MakeMenuItem("Edit", []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						// -- PREFERENCES --
						{ dropDownPadding, MakeMenuItem("Preferences...", []() {
							static std::shared_ptr<SettingsModal> s_settingsModal;
							s_settingsModal = std::make_shared<SettingsModal>();

							auto widget = s_settingsModal->getWidget([]() {
								EditorModalManager::close();
							});

							EditorModalManager::open(widget);
							return Silica::EventReply::handled();
						})},
					}
				})
			})
		});

		// ----- VIEW MENU -----
		auto windowsListContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = dropdownSpacing });

		if (dockspace) {
			std::vector<std::string> availableTabs = dockspace->getRegisteredTabNames();
			for (const std::string& tabName : availableTabs) {
				windowsListContent->addSlot({ dropDownPadding, MakeMenuItem(tabName, [dockspace, tabName]() {
					dockspace->openTab(tabName);
					return Silica::EventReply::handled();
				}) });
			}
		}

		auto windowsSubMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = true,
			.openToRight = true,
			.showArrow = true,
			.anchorContent = MakeMenuItem("Windows", []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = dropDownBg,
				.child = windowsListContent
			})
		});

		auto viewMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.anchorContent = MakeMenuItem("View", []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						// -- View Options --
						{ dropDownPadding, windowsSubMenu }
					}
				})
			})
		});

		// ----- PROJECT MENU -----
		auto projectMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.anchorContent = MakeMenuItem("Project", []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						// -- New Project --
						{ dropDownPadding, MakeMenuItem("New...", []() {
							auto modal = std::make_shared<CreateProjectModal>();
							auto widget = modal->getWidget();
							EditorModalManager::open(widget);
						
							return Silica::EventReply::handled();
						})},
						// -- Open Project --
						{ dropDownPadding, MakeMenuItem("Open...", []() {
							std::filesystem::path filePath = FileDialogs::openFile({ {"Axion Project", "*.axproj"} });

							if (!filePath.empty()) {
								EditorActionQueue::push([filePath]() {
									ProjectManager::loadProject(filePath);
								});
							}
							return Silica::EventReply::handled();
						})},
						// -- Save Project --
						{ dropDownPadding, MakeMenuItem("Save", []() {
							std::filesystem::path filePath = FileDialogs::saveFile({ {"Axion Project", "*.axproj"} });

							if (!filePath.empty()) {
								EditorActionQueue::push([filePath]() {
									ProjectManager::saveProject(filePath);
								});
							}
							return Silica::EventReply::handled();
							
						})},
						// -- Close Project --
						{ dropDownPadding, MakeMenuItem("Close", []() {
							EditorActionQueue::push([]() {
								ProjectManager::unloadProject();
								SceneManager::newScene();
							});
							return Silica::EventReply::handled();
						})},
						// -- Export Project --
						{ dropDownPadding, MakeMenuItem("Export", []() {
							auto modal = std::make_shared<ExportProjectModal>();
							auto widget = modal->getWidget();
							EditorModalManager::open(widget);

							return Silica::EventReply::handled();
						})},
					}
				})
			})
		});

		// ----- HELP MENU -----
		auto helpMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.anchorContent = MakeMenuItem("Help", []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						{ dropDownPadding, MakeMenuItem("System Info", []() {
							auto modal = std::make_shared<SystemInfoModal>();
							auto widget = modal->getWidget();
							EditorModalManager::open(widget);

							return Silica::EventReply::handled();
						})},
					}
				})
			})
		});

		// ----- ASSEMBLE -----
		auto menuBar = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = menuBarBg,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.slots = {
					{ appTitlePadding, Silica::MakeWidget<Silica::STextBlock>({.text = "AXION STUDIO" }) },
					{ menuButtonPadding, fileMenu },
					{ menuButtonPadding, editMenu },
					{ menuButtonPadding, viewMenu },
					{ menuButtonPadding, projectMenu },
					{ menuButtonPadding, helpMenu }
				}
			})
		});

		return menuBar;
	}

}
