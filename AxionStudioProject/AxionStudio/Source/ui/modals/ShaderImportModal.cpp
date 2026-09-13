#include "studiopch.h"
#include "ShaderImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SInputFieldInt.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SButton.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/graphics/Shader.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionAssetPipeline/Source/parser/ShaderParser.h"

namespace Axion {

	ShaderImportModal::ShaderImportModal() {
		m_modalTitle = "Import Shader Asset";
		m_versionText = "v" + std::to_string(ASSET_VERSION_SHADER);
		m_modalWidth = 550.0f;
		resetInputs();
	}

	void ShaderImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();
		m_outputPath = (ProjectManager::getProject()->getAssetsPath() / "shaders").string();

		m_formatIndex = 0;
		m_batchTexturesCount = 1;
	}

	void ShaderImportModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {
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
		contentBox->addSlot({ {0,0}, makePropertyRow("Name", nameInput) });

		// -- Format --
		contentBox->addSlot({ {0,0}, makePropertyRow("Format", makeCombo(m_formatIndex, m_formats)) });

		// -- Batch Textures Count --
		auto batchInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SInputFieldInt>({
				.initialValue = m_batchTexturesCount,
				.onValueChanged = [this](int val) {
					m_batchTexturesCount = std::max(1, val);
				}
			})
		});
		contentBox->addSlot({ {0,0}, makePropertyRow("Batch Textures", batchInput) });

		// -- Source and Output Paths --
		contentBox->addSlot({ {0,0}, makePropertyRow("Source File", makeFileRow(m_sourcePath, "Shader Source", "*.hlsl;*.glsl", "shaders")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Output Location", makeDirectoryRow(m_outputPath, "shaders")) });
	}

	void ShaderImportModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		std::string finalName = m_name + ".axshader";
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

	void ShaderImportModal::onConfirm() {
		std::string finalName = m_name + ".axshader";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		ShaderSpecification spec = {};
		spec.name = m_name;
		spec.batchTextures = m_batchTexturesCount;

		UUID newAssetUUID = UUID::generate();

		AAP::ShaderAssetData data;
		data.uuid = newAssetUUID;
		data.filePath = AssetManager::getRelativeToAssets(m_sourcePath);
		data.fileFormat = AAP::FormatUtils::shaderFormatFromString(m_formats[m_formatIndex]);
		data.spec = spec;

		AAP::ShaderParser::createTextFile(data, finalPath);

		AssetMetadata metadata;
		metadata.handle = newAssetUUID;
		metadata.type = AssetType::Shader;
		metadata.filePath = AssetManager::getRelativeToAssets(finalPath);

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		registry->add(metadata);
		registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
	}

}
