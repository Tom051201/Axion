#include "ExportProjectModal.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionAssetPipeline/Source/core/AssetPackager.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SCheckbox.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorModalManager.h"

namespace Axion {

	Silica::WidgetPtr ExportProjectModal::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(0, 0, 0, 180)
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void ExportProjectModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void ExportProjectModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 12.0f });

		// -- Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Export Project (Windows x64)", .font = m_font}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 2}, .backgroundColor = Silica::Color(80, 80, 80, 255)}) });

		auto project = ProjectManager::getProject();
		if (!project) {
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "No active project to export!", .color = Silica::Color(255, 50, 50, 255), .font = m_font}) });
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
				.padding = { 20.0f, 8.0f }, .color = Silica::Color(80, 80, 80, 255),
				.onClick = [this]() { EditorModalManager::close(); return Silica::EventReply::handled(); },
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Close", .font = m_font})
			}) });
		}
		else {
			// -- Build Summary --
			auto summaryBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 4.0f });
			summaryBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Build Summary:", .color = Silica::Color(100, 200, 100, 255), .font = m_font}) });

			summaryBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({
				.padding = { 15.0f, 0.0f },
				.backgroundColor = Silica::Color::transparent(),
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 2.0f,
					.slots = {
						{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Name: " + project->getName(), .font = m_font}) },
						{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Version: " + project->getVersion().toString(), .font = m_font}) },
						{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Powered by Axion Engine " + Config::EngineVersion.toString(), .color = Silica::Color(150, 150, 150, 255), .font = m_font}) },
						{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = project->getAppIconPath().empty() ? "No Custom Icon (Using Default)" : "Custom Icon: " + project->getAppIconPath().stem().string(), .font = m_font}) },
						{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = project->getDefaultScene().empty() ? "No Default Scene selected!" : "Default Scene: " + project->getDefaultScene().stem().string(), .font = m_font}) }
					}
				})
			}) });

			contentBox->addSlot({ {0,0}, summaryBox });
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 2}, .backgroundColor = Silica::Color(80, 80, 80, 255)}) });

			// -- Helper Function --
			auto MakePropertyRow = [&](const std::string& label, Silica::WidgetPtr valueWidget) {
				return Silica::MakeWidget<Silica::SHorizontalBox>({
					.spacing = 10.0f,
					.slots = {
						{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
							.explicitSize = Silica::Vec2(100.0f, 0.0f),
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


			// -- Export Path --
			auto exportRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {1,0}, Silica::MakeWidget<Silica::SBox>({
						.backgroundColor = Silica::Color(35, 35, 35, 255),
						.child = Silica::MakeWidget<Silica::SEditableText>({
							.initialText = m_exportPath, .font = m_font,
							.onTextChanged = [this](const std::string& val) { m_exportPath = val; rebuildUI(); }
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = {8, 4}, .color = Silica::Color(50, 50, 50, 255),
						.onClick = [this, project]() {
							std::filesystem::path absPath = FileDialogs::openFolder(project->getProjectPath());
							if (!absPath.empty()) { m_exportPath = absPath.string(); rebuildUI(); }
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse...", .font = m_font})
					})}
				}
			});
			contentBox->addSlot({ {0,0}, MakePropertyRow("Export Path", exportRow) });


			// -- Options --
			auto optionsRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::SCheckBox>({
							.initialCheck = m_openAfterExport,
							.onCheckChanged = [this](bool val) { m_openAfterExport = val; rebuildUI(); }
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Open Folder After Export", .font = m_font})
					})}
				}
			});
			contentBox->addSlot({ {0,0}, MakePropertyRow("Options", optionsRow) });


			// -- Validation --
			std::filesystem::path exportDirPath = m_exportPath;
			bool validExportPath = std::filesystem::exists(exportDirPath);
			bool hasDefaultScene = !project->getDefaultScene().empty();
			bool validDefaultScene = std::filesystem::exists(project->getDefaultScene());

			bool disabled = (m_exportPath.empty() || !hasDefaultScene || !validDefaultScene || !validExportPath);

			std::string validationMsg = "Ready to package project.";
			Silica::Color validationColor = Silica::Color(50, 255, 50, 255);

			if (disabled) {
				validationColor = Silica::Color(255, 50, 50, 255);
				if (m_exportPath.empty()) validationMsg = "Export path needs to be set.";
				else if (!validExportPath) validationMsg = "Export directory does not exist.";
				else if (!hasDefaultScene) validationMsg = "Unable to export without a default scene.";
				else if (!validDefaultScene) validationMsg = "Default scene does not exist.";
			}

			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = validationMsg, .color = validationColor, .font = m_font}) });
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 2}, .backgroundColor = Silica::Color(80, 80, 80, 255)}) });


			// -- Footer Buttons --
			auto createBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { 20.0f, 8.0f },
				.color = disabled ? Silica::Color(60, 60, 60, 255) : Silica::Color(50, 150, 50, 255),
				.hoverColor = disabled ? Silica::Color::transparent() : Silica::Color(70, 180, 70, 255),
				.onClick = [this, disabled]() {
					if (disabled) return Silica::EventReply::unhandled();

					AAP::AssetPackager::packageProject(m_exportPath);
					if (m_openAfterExport) PlatformUtils::openFolderInFileExplorer(m_exportPath);

					EditorModalManager::close();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "Package Project",
					.color = disabled ? Silica::Color(120, 120, 120, 255) : Silica::Color::white(),
					.font = m_font,
				})
			});

			auto cancelBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { 20.0f, 8.0f }, .color = Silica::Color(80, 80, 80, 255),
				.onClick = []() {
					EditorModalManager::close();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cancel", .font = m_font})
			});

			auto footerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0,0}, createBtn },
					{ {0,0}, cancelBtn }
				}
			});
			contentBox->addSlot({ {0,0}, footerRow });
		}


		// -- Assemble Modal --
		auto modalPanel = Silica::MakeWidget<Silica::SBox>({
			.padding = { 20.0f, 20.0f },
			.explicitSize = Silica::Vec2{ 550.0f, 0.0f },
			.backgroundColor = Silica::Color(30, 30, 30, 255),
			.child = contentBox
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalPanel
		}));
	}

}
