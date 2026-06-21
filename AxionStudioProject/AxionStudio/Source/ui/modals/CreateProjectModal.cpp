#include "CreateProjectModal.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorModalManager.h"

namespace Axion {

	void CreateProjectModal::resetInputs() {
		m_name.clear();
		m_outputPath.clear();
		m_author.clear();
		m_company.clear();
		m_description.clear();
		m_version = Version(1, 0, 0);
	}

	Silica::WidgetPtr CreateProjectModal::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(0, 0, 0, 180)
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void CreateProjectModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void CreateProjectModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 12.0f });

		// -- Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Create New Project", .font = m_font}) });
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

		// -- Name --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_name, .font = m_font,
				.onTextChanged = [this](const std::string& val) { m_name = val; rebuildUI(); }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Project Name", nameInput) });


		// -- Location --
		auto locRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.backgroundColor = Silica::Color(35, 35, 35, 255),
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_outputPath, .font = m_font,
						.onTextChanged = [this](const std::string& val) { m_outputPath = val; rebuildUI(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4}, .color = Silica::Color(50, 50, 50, 255),
					.onClick = [this]() {
						std::filesystem::path folder = FileDialogs::openFolder();
						if (!folder.empty()) { m_outputPath = folder.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse...", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Location", locRow) });


		// -- Version --
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
				{ {0,0}, makeVersionBox(m_version.major, [this](const std::string& val) { try { m_version.major = std::max(0, std::stoi(val)); } catch (...) {} })},
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = ".", .font = m_font}) },
				{ {0,0}, makeVersionBox(m_version.minor, [this](const std::string& val) { try { m_version.minor = std::max(0, std::stoi(val)); } catch (...) {} })},
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = ".", .font = m_font}) },
				{ {0,0}, makeVersionBox(m_version.patch, [this](const std::string& val) { try { m_version.patch = std::max(0, std::stoi(val)); } catch (...) {} })}
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Version", versionRow) });

		// -- Author --
		auto authInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_author, .font = m_font,
				.onTextChanged = [this](const std::string& val) { m_author = val; }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Author", authInput) });

		// -- Company --
		auto compInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_company, .font = m_font,
				.onTextChanged = [this](const std::string& val) { m_company = val; }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Company", compInput) });

		// -- Description --
		auto descInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_description, .font = m_font,
				.onTextChanged = [this](const std::string& val) { m_description = val; }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Description", descInput) });

		// -- Validation --
		std::filesystem::path outpath = m_outputPath;
		bool validLocation = std::filesystem::is_directory(outpath);
		std::filesystem::path projectFolder = outpath / m_name;
		bool invalidName = std::filesystem::exists(projectFolder);
		bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;

		bool disabled = (m_name.empty() || m_outputPath.empty() || !validLocation || invalidName || nameTooLong);

		std::string validationMsg = "Ready to create project.";
		Silica::Color validationColor = Silica::Color(50, 255, 50, 255);

		if (disabled) {
			validationColor = Silica::Color(255, 50, 50, 255);
			if (m_name.empty()) validationMsg = "Name needs to be set.";
			else if (!validLocation) validationMsg = "Selected Location is not a folder.";
			else if (invalidName) validationMsg = "Project with this name already exists.";
			else if (nameTooLong) validationMsg = "Name exceeds max limit.";
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

				ProjectSpecification spec;
				spec.name = m_name;
				spec.location = m_outputPath;
				spec.author = m_author;
				spec.company = m_company;
				spec.description = m_description;
				spec.version = m_version;

				ProjectManager::newProject(spec);

				EditorModalManager::close();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({
				.text = "Create Project",
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
