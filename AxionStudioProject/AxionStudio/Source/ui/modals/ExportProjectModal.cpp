#include "studiopch.h"
#include "ExportProjectModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SCheckbox.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SButton.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionAssetPipeline/Source/core/AssetPackager.h"
#include "AxionStudio/Source/core/EditorModalManager.h"

namespace Axion {

	ExportProjectModal::ExportProjectModal() {
		m_modalTitle = "Export Project (Windows x64)";
		m_confirmText = "Package Project";
		m_modalWidth = 550.0f;
	}

	Silica::WidgetPtr ExportProjectModal::getWidget() {
		return ModalBase::getWidget([]() {
			EditorModalManager::close();
		});
	}

	void ExportProjectModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {
		auto project = ProjectManager::getProject();

		if (!project) {
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
				.text = "No active project to export!",
				.color = Silica::GetTheme().Text_Danger
			}) });
			return;
		}

		// -- Build Summary --
		auto summaryBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });
		summaryBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Name: " + project->getName() }) });
		summaryBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Version: " + project->getVersion().toString() }) });
		summaryBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = "Powered by Axion Engine " + Config::EngineVersion.toString(),
			.color = Silica::GetTheme().Text_Dim
		}) });
		summaryBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = project->getAppIconPath().empty() ? "No Custom Icon (Using Default)" : "Custom Icon: " + project->getAppIconPath().stem().string()
		}) });
		summaryBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = project->getDefaultScene().empty() ? "No Default Scene selected!" : "Default Scene: " + project->getDefaultScene().stem().string()
		}) });

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Build Summary:", .color = Silica::GetTheme().Text_Success }) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.padding = { 15.0f, 0.0f }, .child = summaryBox }) });

		// -- Export Path --
		auto exportRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_exportPath,
						.onTextChanged = [this](const std::string& val) { m_exportPath = val; validate(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this, project]() {
						std::filesystem::path absPath = FileDialogs::openFolder(project->getProjectPath());
						if (!absPath.empty()) { m_exportPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
				})}
			}
		});
		contentBox->addSlot({ {0,0}, makePropertyRow("Export Path", exportRow) });

		// -- Options --
		auto optionsRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::SCheckBox>({
						.initialCheck = m_openAfterExport,
						.onCheckChanged = [this](bool val) { m_openAfterExport = val; }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Open Folder After Export" })
				})}
			}
		});
		contentBox->addSlot({ {0,0}, makePropertyRow("Options", optionsRow) });
	}

	void ExportProjectModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		auto project = ProjectManager::getProject();
		if (!project) {
			m_confirmBtn->setEnabled(false);
			return;
		}

		bool validExportPath = false;
		bool validDefaultScene = false;
		bool hasDefaultScene = !project->getDefaultScene().empty();
		std::error_code ec;

		if (!m_exportPath.empty()) validExportPath = std::filesystem::exists(m_exportPath, ec);
		if (hasDefaultScene) validDefaultScene = std::filesystem::exists(project->getDefaultScene(), ec);

		bool disabled = (m_exportPath.empty() || !hasDefaultScene || !validDefaultScene || !validExportPath);

		std::string validationMsg = "Ready to package project.";
		Silica::Color validationColor = Silica::GetTheme().Text_Success;

		if (disabled) {
			validationColor = Silica::GetTheme().Text_Danger;
			if (m_exportPath.empty()) validationMsg = "Export path needs to be set.";
			else if (!validExportPath) validationMsg = "Export directory does not exist.";
			else if (!hasDefaultScene) validationMsg = "Unable to export without a default scene.";
			else if (!validDefaultScene) validationMsg = "Default scene does not exist.";
		}

		m_validationText->setText(validationMsg);
		m_validationText->setColor(validationColor);
		m_confirmBtn->setEnabled(!disabled);
	}

	void ExportProjectModal::onConfirm() {
		AAP::AssetPackager::packageProject(m_exportPath);
		if (m_openAfterExport) PlatformUtils::openFolderInFileExplorer(m_exportPath);
		EditorModalManager::close();
	}

}
