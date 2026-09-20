#include "studiopch.h"
#include "TextureCubeImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SButton.h>
#include <Silica/include/SSpacer.h>
#include <Silica/include/SSeparator.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/TextureCubeParser.h"

#include "AxionStudio/Source/ui/SilicaHelpers.h"

namespace Axion {

	TextureCubeImportModal::TextureCubeImportModal() {
		m_modalTitle = "Import TextureCube Asset";
		m_versionText = "v" + std::to_string(ASSET_VERSION_TEXTURE_CUBE);
		m_modalWidth = 550.0f;
		resetInputs();
	}

	void TextureCubeImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();

		m_outputPath.clear();
		std::filesystem::path texDir = ProjectManager::getProject()->getAssetsPath() / "Textures";
		if (std::filesystem::exists(texDir)) m_outputPath = texDir.generic_string();

		m_importType = 0;
	}

	void TextureCubeImportModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {
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
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Type", makeCombo(m_importType, m_types)) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.space = EditorTheme::SPACING_LARGE}) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Source File", makeFileRow(m_sourcePath, "Image File", "*.png;*.jpg;*.jpeg", "textures")) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Output Location", makeDirectoryRow(m_outputPath, "textures")) });
	}

	void TextureCubeImportModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		std::string finalName = m_name + ".axtcube";
		std::filesystem::path finalPath;

		bool sourceExists = false;
		bool sourceIsFile = false;
		bool outputExists = false;
		bool outputIsDirectory = false;
		bool invalidOutFileName = false;

		// -- Safe Filesystem Checks --
		try {
			std::error_code ec;
			if (!m_sourcePath.empty()) {
				sourceExists = std::filesystem::exists(m_sourcePath, ec);
				sourceIsFile = std::filesystem::is_regular_file(m_sourcePath, ec);
			}
			if (!m_outputPath.empty()) {
				outputExists = std::filesystem::exists(m_outputPath, ec);
				outputIsDirectory = std::filesystem::is_directory(m_outputPath, ec);
				finalPath = std::filesystem::path(m_outputPath) / finalName;
				invalidOutFileName = std::filesystem::exists(finalPath, ec);
			}
		}
		catch (...) {}

		bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;

		bool disabled = (m_name.empty() || m_sourcePath.empty() || m_outputPath.empty() || !sourceExists || !sourceIsFile || !outputExists || !outputIsDirectory || invalidOutFileName || nameTooLong);

		std::string validationMsg = "Ready to create asset.";
		Silica::Color validationColor = Silica::GetTheme().Text_Success;

		if (disabled) {
			validationColor = Silica::GetTheme().Text_Danger;
			if (m_name.empty()) validationMsg = "No Name is set.";
			else if (m_sourcePath.empty()) validationMsg = "No source file is set.";
			else if (m_outputPath.empty()) validationMsg = "No output directory is set.";
			else if (!sourceExists) validationMsg = "Source file does not exist.";
			else if (!sourceIsFile) validationMsg = "Source is not a file.";
			else if (!outputExists) validationMsg = "Output directory does not exist.";
			else if (!outputIsDirectory) validationMsg = "Output is not a directory.";
			else if (invalidOutFileName) validationMsg = "Asset with this name already exists.";
			else if (nameTooLong) validationMsg = "Name exceeds max limit.";
		}

		m_validationText->setText(validationMsg);
		m_validationText->setColor(validationColor);
		m_confirmBtn->setEnabled(!disabled);
	}

	void TextureCubeImportModal::onConfirm() {
		std::string finalName = m_name + ".axtcube";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		UUID newAssetUUID = UUID::generate();

		AAP::TextureCubeAssetData data;
		data.uuid = newAssetUUID;
		data.name = m_name;
		data.fileFormat = AAP::FormatUtils::textureFormatFromString(m_types[m_importType]);
		data.filePath = AssetManager::getRelativeToAssets(m_sourcePath);

		AAP::TextureCubeParser::createTextFile(data, finalPath);

		AssetMetadata metadata;
		metadata.handle = newAssetUUID;
		metadata.type = AssetType::TextureCube;
		metadata.filePath = AssetManager::getRelativeToAssets(finalPath);

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		registry->add(metadata);
		registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
	}

}
