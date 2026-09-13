#include "studiopch.h"
#include "EditorMenuBar.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SButton.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SDockSpace.h>
#include <Silica/include/Renderer.h>

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorCommand.h"

namespace {
	constexpr float DROPDOWN_SPACING = 1.0f;
	constexpr Silica::Vec2 DROPDOWN_PADDING = { 0.0f, 0.0f };
	constexpr Silica::Vec2 MENU_BTN_PADDING = { 0.0f, 2.0f };
	constexpr Silica::Vec2 APP_TITLE_PADDING = { 25.0f, 6.0f };
	constexpr Silica::Vec2 MENU_ITEM_PADDING = { 12.0f, 4.0f };

	// ----- Helper Functions -----
	Silica::WidgetPtr MakeMenuItem(const std::string& text, std::function<Silica::EventReply()> onClick, bool closeOverlays = true) {
		return Silica::MakeWidget<Silica::SButton>({
			.padding = MENU_ITEM_PADDING,
			.color = Silica::GetTheme().Surface_Tertiary,
			.onClick = [onClick, closeOverlays]() {
				if (closeOverlays) {
					Silica::Renderer::closeAllOverlays();
				}
				return onClick();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = text })
		});
	}
}

namespace Axion {

	Silica::WidgetPtr EditorMenuBar::construct(std::shared_ptr<Silica::SDockSpace> dockspace, const MenuBarCallbacks& callbacks) {
		Silica::Color menuBarBg = Silica::GetTheme().Surface_Tertiary;
		Silica::Color dropDownBg = Silica::GetTheme().Surface_Tertiary;

		// ----- FILE MENU -----
		auto fileMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.hoverGroup = "MainMenuBar",
			.anchorContent = MakeMenuItem("File", []() { return Silica::EventReply::unhandled(); }, false),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = DROPDOWN_SPACING,
					.slots = {
						// -- NEW SCENE --
						{ DROPDOWN_PADDING, MakeMenuItem("New Scene", [callbacks]() {
							if (callbacks.newScene) callbacks.newScene();
							return Silica::EventReply::handled();
						}) },

						// -- LOAD SCENE --
						{ DROPDOWN_PADDING, MakeMenuItem("Load Scene", [callbacks]() {
							if (callbacks.openScene) callbacks.openScene();
							return Silica::EventReply::handled();
						}) },

						// -- SAVE SCENE --
						{ DROPDOWN_PADDING, MakeMenuItem("Save Scene", [callbacks]() {
							if (callbacks.saveScene) callbacks.saveScene();
							return Silica::EventReply::handled();
						}) },

						// -- SAVE SCENE AS --
						{ DROPDOWN_PADDING, MakeMenuItem("Save Scene As...", [callbacks]() {
							if (callbacks.saveSceneAs) callbacks.saveSceneAs();
							return Silica::EventReply::handled();
						}) },

						// -- EXIT --
						{ DROPDOWN_PADDING, MakeMenuItem("Exit", [callbacks]() {
							if (callbacks.exitEditor) callbacks.exitEditor();
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
			.hoverGroup = "MainMenuBar",
			.anchorContent = MakeMenuItem("Edit", []() { return Silica::EventReply::unhandled(); }, false),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = DROPDOWN_SPACING,
					.slots = {
						// -- UNDO / REDO --
						{ DROPDOWN_PADDING, MakeMenuItem("Undo", []() {
							EditorCommandManager::undo();
							return Silica::EventReply::handled();
						})},
						{ DROPDOWN_PADDING, MakeMenuItem("Redo", []() {
							EditorCommandManager::redo();
							return Silica::EventReply::handled();
						})},
						// -- PREFERENCES --
						{ DROPDOWN_PADDING, MakeMenuItem("Preferences...", [callbacks]() {
							if (callbacks.openPreferences) {
								callbacks.openPreferences();
							}
							return Silica::EventReply::handled();
						})},
					}
				})
			})
		});

		// ----- VIEW MENU -----
		auto windowsListContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = DROPDOWN_SPACING });

		if (dockspace) {
			std::vector<std::string> availableTabs = dockspace->getRegisteredTabNames();
			for (const std::string& tabName : availableTabs) {
				windowsListContent->addSlot({ DROPDOWN_PADDING, MakeMenuItem(tabName, [dockspace, tabName]() {
					dockspace->openTab(tabName);
					dockspace->focusTab(tabName);
					return Silica::EventReply::handled();
				}) });
			}
		}

		auto windowsSubMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = true,
			.openToRight = true,
			.showArrow = true,
			.anchorContent = MakeMenuItem("Windows", []() { return Silica::EventReply::unhandled(); }, false),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = dropDownBg,
				.child = windowsListContent
			})
		});

		auto viewMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.hoverGroup = "MainMenuBar",
			.anchorContent = MakeMenuItem("View", []() { return Silica::EventReply::unhandled(); }, false),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = DROPDOWN_SPACING,
					.slots = {
						// -- View Options --
						{ DROPDOWN_PADDING, windowsSubMenu }
					}
				})
			})
		});

		windowsSubMenu->setParentMenu(viewMenu.get());

		// ----- PROJECT MENU -----
		auto projectMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.hoverGroup = "MainMenuBar",
			.anchorContent = MakeMenuItem("Project", []() { return Silica::EventReply::unhandled(); }, false),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = DROPDOWN_SPACING,
					.slots = {
						// -- New Project --
						{ DROPDOWN_PADDING, MakeMenuItem("New...", [callbacks]() {
							if (callbacks.openCreateProjectModal) callbacks.openCreateProjectModal();
							return Silica::EventReply::handled();
						})},
						// -- Open Project --
						{ DROPDOWN_PADDING, MakeMenuItem("Open...", []() {
							std::filesystem::path filePath = FileDialogs::openFile({ {"Axion Project", "*.axproj"} });

							if (!filePath.empty()) {
								EditorActionQueue::push([filePath]() {
									ProjectManager::loadProject(filePath);
								});
							}
							return Silica::EventReply::handled();
						})},
						// -- Save Project --
						{ DROPDOWN_PADDING, MakeMenuItem("Save", []() {
							std::filesystem::path filePath = FileDialogs::saveFile({ {"Axion Project", "*.axproj"} });

							if (!filePath.empty()) {
								EditorActionQueue::push([filePath]() {
									ProjectManager::saveProject(filePath);
								});
							}
							return Silica::EventReply::handled();
						})},
						// -- Close Project --
						{ DROPDOWN_PADDING, MakeMenuItem("Close", []() {
							EditorActionQueue::push([]() {
								ProjectManager::unloadProject();
								SceneManager::newScene();
							});
							return Silica::EventReply::handled();
						})},
						// -- Export Project --
						{ DROPDOWN_PADDING, MakeMenuItem("Export", [callbacks]() {
							if (callbacks.openExportProjectModal) callbacks.openExportProjectModal();
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
			.hoverGroup = "MainMenuBar",
			.anchorContent = MakeMenuItem("Help", []() { return Silica::EventReply::unhandled(); }, false),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = DROPDOWN_SPACING,
					.slots = {
						// -- System Info --
						{ DROPDOWN_PADDING, MakeMenuItem("System Info", [callbacks]() {
							if (callbacks.openSystemInfoModal) callbacks.openSystemInfoModal();
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
					{ APP_TITLE_PADDING, Silica::MakeWidget<Silica::STextBlock>({.text = "AXION STUDIO" }) },
					{ MENU_BTN_PADDING, fileMenu },
					{ MENU_BTN_PADDING, editMenu },
					{ MENU_BTN_PADDING, viewMenu },
					{ MENU_BTN_PADDING, projectMenu },
					{ MENU_BTN_PADDING, helpMenu }
				}
			})
		});

		return menuBar;
	}

}
