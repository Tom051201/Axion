#include "studiopch.h"
#include "AudioImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SButton.h>
#include <Silica/include/SSpacer.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/audio/AudioManager.h"

#include "AxionAssetPipeline/Source/parser/AudioParser.h"

#include "AxionStudio/Source/ui/SilicaHelpers.h"

namespace Axion {

	AudioImportModal::AudioImportModal() {
		m_modalTitle = "Import Audio Asset";
		m_versionText = "v" + std::to_string(ASSET_VERSION_AUDIO);
		m_modalWidth = 550.0f;
		resetInputs();
	}

	void AudioImportModal::presetFromFile(const std::filesystem::path& sourceFile) {
		resetInputs();

		m_sourcePath = sourceFile.generic_string();

		std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "Audio";
		if (std::filesystem::exists(audioDir)) { m_outputPath = audioDir.generic_string(); }

		// -- Modify name --
		std::string rawName = sourceFile.stem().string();
		if (!rawName.empty()) {
			rawName[0] = std::toupper(static_cast<unsigned char>(rawName[0]));
			std::replace(rawName.begin(), rawName.end(), ' ', '_');
		}
		m_name = rawName;

		AudioFileInfo fileInfo;
		bool success = AudioManager::readAudioFileMetadata(sourceFile, fileInfo);
		AudioClip::Mode mode = AudioClip::Mode::Stream;
		if (success) {
			mode = AudioManager::decideMode(fileInfo);
			if (mode == AudioClip::Mode::Memory) m_loadType = 1;
		}

		std::string formatStr = sourceFile.extension().string();
		std::transform(formatStr.begin(), formatStr.end(), formatStr.begin(), [](unsigned char c) { return std::tolower(c); });
		if (formatStr == ".mp3") m_importFormat = 0;
		else if (formatStr == ".wav") m_importFormat = 1;
		else if (formatStr == ".ogg") m_importFormat = 2;
		else AX_CORE_LOG_WARN("Unable to identify automatically format of audio");
	}

	void AudioImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();
		m_outputPath.clear();
		m_loadType = 0;
		m_importFormat = 0;
	}

	void AudioImportModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {
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
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Format", makeCombo(m_importFormat, m_formatNames)) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Type", makeCombo(m_loadType, m_typesNames)) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.space = EditorTheme::SPACING_LARGE}) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Source File", makeFileRow(m_sourcePath, "Audio Files", "*.mp3;*.wav;*.ogg", "audio")) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Output Location", makeDirectoryRow(m_outputPath, "audio")) });
	}

	void AudioImportModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		bool sourceExists = false;
		bool sourceIsFile = false;
		bool outputExists = false;
		bool outputIsDirectory = false;
		bool invalidOutFileName = false;

		std::string finalName = m_name + ".axaudio";
		std::filesystem::path finalPath;

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

	void AudioImportModal::onConfirm() {
		std::string finalName = m_name + ".axaudio";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		UUID newAssetUUID = UUID::generate();
		AAP::AudioAssetData data;
		data.uuid = newAssetUUID;
		data.name = m_name;
		data.fileFormat = AAP::FormatUtils::audioFormatFromString(m_formatNames[m_importFormat]);
		data.audioFilePath = AssetManager::getRelativeToAssets(m_sourcePath);
		data.mode = m_types[m_loadType];

		AAP::AudioParser::createTextFile(data, finalPath);

		AssetMetadata metadata;
		metadata.handle = newAssetUUID;
		metadata.type = AssetType::AudioClip;
		metadata.filePath = AssetManager::getRelativeToAssets(finalPath);

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		registry->add(metadata);
		registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
	}

}
