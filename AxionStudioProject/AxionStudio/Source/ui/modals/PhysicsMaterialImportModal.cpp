#include "studiopch.h"
#include "PhysicsMaterialImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/SSpacer.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/PhysicsMaterialParser.h"

#include "AxionStudio/Source/ui/SilicaHelpers.h"

namespace Axion {

	PhysicsMaterialImportModal::PhysicsMaterialImportModal() {
		m_modalTitle = "Import Physics Material";
		m_versionText = "v" + std::to_string(ASSET_VERSION_PHYSICS_MATERIAL);
		m_modalWidth = 500.0f;
		resetInputs();
	}

	void PhysicsMaterialImportModal::resetInputs() {
		m_name.clear();

		m_outputPath.clear();
		std::filesystem::path phyDir = ProjectManager::getProject()->getAssetsPath() / "Physics";
		if (std::filesystem::exists(phyDir)) m_outputPath = phyDir.generic_string();

		m_staticFriction = 0.5f;
		m_dynamicFriction = 0.5f;
		m_restitution = 0.05f;
	}

	void PhysicsMaterialImportModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {
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
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Name", nameInput) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSpacer>({.size = {0.0f, EditorTheme::SPACING_SMALL} }) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Static Friction", makeSliderRow(m_staticFriction, 10.0f)) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Dynamic Friction", makeSliderRow(m_dynamicFriction, 10.0f)) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Restitution", makeSliderRow(m_restitution, 1.0f)) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.space = EditorTheme::SPACING_LARGE}) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Output Location", makeDirectoryRow(m_outputPath, "physics")) });
	}

	void PhysicsMaterialImportModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		std::string finalName = m_name + ".axpmat";
		std::filesystem::path finalPath;

		bool outputExists = false;
		bool outputIsDirectory = false;
		bool invalidOutFileName = false;

		// -- Safe Filesystem Checks --
		try {
			std::error_code ec;
			if (!m_outputPath.empty()) {
				outputExists = std::filesystem::exists(m_outputPath, ec);
				outputIsDirectory = std::filesystem::is_directory(m_outputPath, ec);
				finalPath = std::filesystem::path(m_outputPath) / finalName;
				invalidOutFileName = std::filesystem::exists(finalPath, ec);
			}
		}
		catch (...) {}

		bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;

		bool disabled = (m_name.empty() || m_outputPath.empty() || !outputExists || !outputIsDirectory || invalidOutFileName || nameTooLong);

		std::string validationMsg = "Ready to create asset.";
		Silica::Color validationColor = Silica::GetTheme().Text_Success;

		if (disabled) {
			validationColor = Silica::GetTheme().Text_Danger;
			if (m_name.empty()) validationMsg = "Name needs to be set.";
			else if (m_outputPath.empty()) validationMsg = "No output directory is set.";
			else if (!outputExists) validationMsg = "Output directory does not exist.";
			else if (!outputIsDirectory) validationMsg = "Output is not a directory.";
			else if (invalidOutFileName) validationMsg = "Asset with this name already exists.";
			else if (nameTooLong) validationMsg = "Name exceeds max limit.";
		}

		m_validationText->setText(validationMsg);
		m_validationText->setColor(validationColor);
		m_confirmBtn->setEnabled(!disabled);
	}

	void PhysicsMaterialImportModal::onConfirm() {
		std::string finalName = m_name + ".axpmat";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		UUID newAssetUUID = UUID::generate();

		AAP::PhysicsMaterialAssetData data;
		data.uuid = newAssetUUID;
		data.name = m_name;
		data.staticFriction = m_staticFriction;
		data.dynamicFriction = m_dynamicFriction;
		data.restitution = m_restitution;

		AAP::PhysicsMaterialParser::createTextFile(data, finalPath);

		AssetMetadata metadata;
		metadata.handle = newAssetUUID;
		metadata.type = AssetType::PhysicsMaterial;
		metadata.filePath = AssetManager::getRelativeToAssets(finalPath);

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		registry->add(metadata);
		registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
	}

}
