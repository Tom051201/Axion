#include "studiopch.h"
#include "SkyboxImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SButton.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/SkyboxParser.h"

namespace Axion {

	SkyboxImportModal::SkyboxImportModal() {
		m_modalTitle = "Import Skybox Asset";
		m_versionText = "v" + std::to_string(ASSET_VERSION_SKYBOX);
		m_modalWidth = 550.0f;
		resetInputs();
	}

	void SkyboxImportModal::resetInputs() {
		m_name.clear();
		m_texturePath.clear();
		m_pipelinePath.clear();

		std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "skybox";
		m_outputPath = dir.string();
	}

	void SkyboxImportModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {
		// -- Name --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_name ,
				.onTextChanged = [this](const std::string& val) {
					m_name = val;
					validate();
				}
			})
		});
		contentBox->addSlot({ {0,0}, makePropertyRow("Name", nameInput) });

		contentBox->addSlot({ {0,0}, makePropertyRow("Texture Cube", makeFileRow(m_texturePath, "Axion Texture File", "*.axtcube", "textures")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Pipeline", makeFileRow(m_pipelinePath, "Axion Pipeline Asset", "*.axpso", "pipelines")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Output Location", makeDirectoryRow(m_outputPath, "skybox")) });
	}

	void SkyboxImportModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		std::string finalName = m_name + ".axsky";
		std::filesystem::path finalPath;

		bool hasPipeline = !m_pipelinePath.empty();
		bool pipelineExists = true;
		bool pipelineIsFile = true;
		bool textureExists = false;
		bool textureIsFile = false;
		bool outputExists = false;
		bool outputIsDirectory = false;
		bool invalidOutFileName = false;

		// -- Safe Filesystem Checks --
		try {
			std::error_code ec;
			if (hasPipeline) {
				pipelineExists = std::filesystem::exists(m_pipelinePath, ec);
				pipelineIsFile = std::filesystem::is_regular_file(m_pipelinePath, ec);
			}
			if (!m_texturePath.empty()) {
				textureExists = std::filesystem::exists(m_texturePath, ec);
				textureIsFile = std::filesystem::is_regular_file(m_texturePath, ec);
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

		bool disabled = (m_name.empty() || m_texturePath.empty() || m_outputPath.empty() || !pipelineExists || !pipelineIsFile || !textureExists || !textureIsFile || !outputExists || !outputIsDirectory || invalidOutFileName || nameTooLong);

		std::string validationMsg = "Ready to create asset.";
		Silica::Color validationColor = Silica::GetTheme().Text_Success;

		if (disabled) {
			validationColor = Silica::GetTheme().Text_Danger;
			if (m_name.empty()) validationMsg = "No Name is set.";
			else if (m_texturePath.empty()) validationMsg = "No texture file is set.";
			else if (m_outputPath.empty()) validationMsg = "No output directory is set.";
			else if (!pipelineExists) validationMsg = "Pipeline file does not exist.";
			else if (!pipelineIsFile) validationMsg = "Pipeline is not a file.";
			else if (!textureExists) validationMsg = "Texture file does not exist.";
			else if (!textureIsFile) validationMsg = "Texture is not a file.";
			else if (!outputExists) validationMsg = "Output directory does not exist.";
			else if (!outputIsDirectory) validationMsg = "Output is not a directory.";
			else if (invalidOutFileName) validationMsg = "Asset with this name already exists.";
			else if (nameTooLong) validationMsg = "Name exceeds max limit.";
		}

		m_validationText->setText(validationMsg);
		m_validationText->setColor(validationColor);
		m_confirmBtn->setEnabled(!disabled);
	}

	void SkyboxImportModal::onConfirm() {
		std::string finalName = m_name + ".axsky";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;
		bool hasPipeline = !m_pipelinePath.empty();

		UUID newAssetUUID = UUID::generate();

		AAP::SkyboxAssetData data;
		data.uuid = newAssetUUID;
		data.name = m_name;
		data.textureCubePath = AssetManager::getRelativeToAssets(m_texturePath);
		if (hasPipeline) {
			data.pipelinePath = AssetManager::getRelativeToAssets(m_pipelinePath);
		}

		AAP::SkyboxParser::createTextFile(data, finalPath);

		AssetMetadata metadata;
		metadata.handle = newAssetUUID;
		metadata.type = AssetType::Skybox;
		metadata.filePath = AssetManager::getRelativeToAssets(finalPath);

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		registry->add(metadata);
		registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
	}

}
