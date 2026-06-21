#include "EditorMenuBar.h"

#include "AxionEngine/Source/core/Logging.h"

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
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorModalManager.h"

namespace Axion {

	constexpr Silica::Color menuBarCBg(40, 40, 40, 255);
	constexpr Silica::Color dropDownBg(45, 45, 45, 255);
	constexpr float dropdownSpacing = 0.0f;
	constexpr Silica::Vec2 dropDownPadding = { 0.0f, 0.0f };
	constexpr Silica::Vec2 menuButtonPadding = { 0.0f, 2.0f };
	constexpr Silica::Vec2 appTitlePadding = { 15.0f, 6.0f };


	// ----- Helper Functions -----
	Silica::WidgetPtr MakeMenuItem(const std::string& text, Silica::FontAtlas* font, std::function<Silica::EventReply()> onClick) {
		return Silica::MakeWidget<Silica::SButton>({
			.padding = { 12.0f, 4.0f },
			.color = dropDownBg,
			.hoverColor = Silica::GetTheme().buttonHover,
			.pressedColor = Silica::GetTheme().buttonPressed,
			.onClick = onClick,
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = text, .font = font })
		});
	}



	Silica::WidgetPtr EditorMenuBar::construct(Silica::FontAtlas* font) {
		// ----- IMPORT SUB MENU -----
		auto importMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = true,
			.openToRight = true,
			.showArrow = true,
			.anchorContent = MakeMenuItem("Import", font, []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						{ dropDownPadding, MakeMenuItem("Mesh", font, []() { return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Texture 2D", font, []() { return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Texture Cube", font, []() { return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Material", font, []() { return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Skybox", font, []() { return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Shader", font, []() { return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Pipeline", font, []() { return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Audio", font, []() { return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Physics Material", font, []() { return Silica::EventReply::handled(); }) }
					}
				})
			})
		});

		// ----- FILE MENU -----
		auto fileMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.anchorContent = MakeMenuItem("File", font, []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						{ dropDownPadding, MakeMenuItem("New Scene", font, []() { AX_CORE_LOG_TRACE("New"); return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Load Scene", font, []() { AX_CORE_LOG_TRACE("Load"); return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Save Scene", font, []() { AX_CORE_LOG_TRACE("Save"); return Silica::EventReply::handled(); }) },
						{ dropDownPadding, MakeMenuItem("Save Scene As...", font, []() { AX_CORE_LOG_TRACE("Save As"); return Silica::EventReply::handled(); }) },
						{ dropDownPadding, importMenu },
						{ dropDownPadding, MakeMenuItem("Exit", font, []() { AX_CORE_LOG_TRACE("Exit"); return Silica::EventReply::handled(); }) }
					}
				})
			}),
		});

		// ----- EDIT MENU -----
		auto editMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.anchorContent = MakeMenuItem("Edit", font, []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						{ dropDownPadding, MakeMenuItem("Nothing here...", font, []() { return Silica::EventReply::handled(); })},
					}
				})
			})
		});

		// ----- VIEW MENU -----
		auto viewMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.anchorContent = MakeMenuItem("View", font, []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						{ dropDownPadding, MakeMenuItem("Nothing here...", font, []() { return Silica::EventReply::handled(); })},
					}
				})
			})
		});

		// ----- PROJECT MENU -----
		auto projectMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.openToRight = false,
			.anchorContent = MakeMenuItem("Project", font, []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						{ dropDownPadding, MakeMenuItem("New...", font, [font]() {
							auto modal = std::make_shared<CreateProjectModal>();
							auto widget = modal->getWidget(font);
							EditorModalManager::open(widget);
						
							return Silica::EventReply::handled();
						})},
						{ dropDownPadding, MakeMenuItem("Open...", font, []() { AX_CORE_LOG_TRACE("Add Open Proj func"); return Silica::EventReply::handled(); })},
						{ dropDownPadding, MakeMenuItem("Save", font, []() { AX_CORE_LOG_TRACE("Add Save Proj func"); return Silica::EventReply::handled(); })},
						{ dropDownPadding, MakeMenuItem("Close", font, []() { AX_CORE_LOG_TRACE("Add Close Proj func"); return Silica::EventReply::handled(); })},
						{ dropDownPadding, MakeMenuItem("Export", font, [font]() {
							auto modal = std::make_shared<ExportProjectModal>();
							auto widget = modal->getWidget(font);
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
			.anchorContent = MakeMenuItem("Help", font, []() { return Silica::EventReply::unhandled(); }),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = dropDownBg,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = dropdownSpacing,
					.slots = {
						{ dropDownPadding, MakeMenuItem("System Info", font, [font]() {
							auto modal = std::make_shared<SystemInfoModal>();
							auto widget = modal->getWidget(font);
							EditorModalManager::open(widget);

							return Silica::EventReply::handled();
						})},
					}
				})
			})
		});

		// ----- ASSEMBLE -----
		auto menuBar = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = menuBarCBg,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.slots = {
					{ appTitlePadding, Silica::MakeWidget<Silica::STextBlock>({.text = "AXION STUDIO", .font = font}) },
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
