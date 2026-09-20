#include "studiopch.h"
#include "CreateProjectModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SInputFieldInt.h>
#include <Silica/include/SSeparator.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionStudio/Source/core/EditorModalManager.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"
#include "AxionStudio/Source/ui/EditorTheme.h"

namespace Axion {

	// TODO: fix magic numbers in here and match it to the other modals

	CreateProjectModal::CreateProjectModal() {
		m_modalTitle = "Create New Project";
		m_confirmText = "Create Project";
		m_modalWidth = 550.0f;
		resetInputs();
	}

	Silica::WidgetPtr CreateProjectModal::getWidget() {
		return ModalBase::getWidget([]() { EditorModalManager::close(); });
	}

	void CreateProjectModal::resetInputs() {
		m_name.clear();
		m_outputPath.clear();
		m_author.clear();
		m_company.clear();
		m_description.clear();
		m_version = Version(1, 0, 0);
	}

	void CreateProjectModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {
		// -- Name --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_name,
				.onTextChanged = [this](const std::string& val) {
					m_name = val;
					validate();
				}
			})
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Project Name", nameInput) });

		// -- Location --
		auto locRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_outputPath,
						.onTextChanged = [this](const std::string& val) { m_outputPath = val; validate(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this]() {
						std::filesystem::path folder = FileDialogs::openFolder();
						if (!folder.empty()) {
							m_outputPath = folder.generic_string();
							rebuildUI();
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
				})}
			}
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Location", locRow) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.space = EditorTheme::SPACING_LARGE}) });

		// -- Version --
		auto makeVersionBox = [this](uint32_t& versionRef) {
			return Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2(50.0f, 0.0f),
				.child = Silica::MakeWidget<Silica::SInputFieldInt>({
					.initialValue = (int)versionRef,
					.onValueChanged = [&versionRef](int val) { versionRef = std::max(0, val); }
				})
			});
		};

		auto versionRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 5.0f,
			.slots = {
				{ {0,0}, makeVersionBox(m_version.major) },
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "." }) },
				{ {0,0}, makeVersionBox(m_version.minor) },
				{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "." }) },
				{ {0,0}, makeVersionBox(m_version.patch) }
			}
		});
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Version", versionRow) });

		// -- Author, Company, Description --
		auto makeTextRow = [this](const std::string& label, std::string& stringRef) {
			auto input = Silica::MakeWidget<Silica::SBox>({
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = stringRef,
					.onTextChanged = [&stringRef](const std::string& val) { stringRef = val; }
				})
			});
			return SilicaHelpers::MakePropertyRow(label, input);
		};

		contentBox->addSlot({ {0,0}, makeTextRow("Author", m_author) });
		contentBox->addSlot({ {0,0}, makeTextRow("Company", m_company) });
		contentBox->addSlot({ {0,0}, makeTextRow("Description", m_description) });
	}

	void CreateProjectModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		bool validLocation = false;
		bool invalidName = false;
		std::error_code ec;

		if (!m_outputPath.empty()) {
			validLocation = std::filesystem::is_directory(m_outputPath, ec);
		}

		if (validLocation && !m_name.empty()) {
			invalidName = std::filesystem::exists(std::filesystem::path(m_outputPath) / m_name, ec);
		}

		bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;
		bool disabled = (m_name.empty() || m_outputPath.empty() || !validLocation || invalidName || nameTooLong);

		std::string validationMsg = "Ready to create project.";
		Silica::Color validationColor = Silica::GetTheme().Text_Success;

		if (disabled) {
			validationColor = Silica::GetTheme().Text_Danger;
			if (m_name.empty()) validationMsg = "Name needs to be set.";
			else if (m_outputPath.empty()) validationMsg = "Location needs to be set.";
			else if (!validLocation) validationMsg = "Selected Location is not a folder.";
			else if (invalidName) validationMsg = "Project with this name already exists.";
			else if (nameTooLong) validationMsg = "Name exceeds max limit.";
		}

		m_validationText->setText(validationMsg);
		m_validationText->setColor(validationColor);
		m_confirmBtn->setEnabled(!disabled);
	}

	void CreateProjectModal::onConfirm() {
		ProjectSpecification spec;
		spec.name = m_name;
		spec.location = m_outputPath;
		spec.author = m_author;
		spec.company = m_company;
		spec.description = m_description;
		spec.version = m_version;

		ProjectManager::newProject(spec);
		EditorModalManager::close();
	}
}
