#include "studiopch.h"
#include "MeshImportModal.h"

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

#include "AxionAssetPipeline/Source/parser/MeshParser.h"

namespace Axion {

	MeshImportModal::MeshImportModal() {
		m_modalTitle = "Import Mesh Asset";
		m_versionText = "v" + std::to_string(ASSET_VERSION_MESH);
		m_modalWidth = 550.0f;
		resetInputs();
	}

	void MeshImportModal::presetFromFile(const std::filesystem::path& sourceFile) {
		resetInputs();

		m_sourcePath = sourceFile.string();
		std::filesystem::path meshDir = ProjectManager::getProject()->getAssetsPath() / "meshes";
		m_outputPath = meshDir.string();
		m_name = sourceFile.stem().string();

		std::string typeStr = sourceFile.extension().string();
		std::transform(typeStr.begin(), typeStr.end(), typeStr.begin(), [](unsigned char c) { return std::tolower(c); });

		if (typeStr == ".obj") m_importType = 0;
		else if (typeStr == ".gltf") m_importType = 1;
		else if (typeStr == ".glb") m_importType = 2;
		else AX_CORE_LOG_WARN("Unable to identify automatically type of mesh");
	}

	void MeshImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();
		std::filesystem::path meshDir = ProjectManager::getProject()->getAssetsPath() / "meshes";
		m_outputPath = meshDir.string();
		m_importType = 0;
	}

	void MeshImportModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {
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

		// -- Type --
		contentBox->addSlot({ {0,0}, makePropertyRow("Type", makeCombo(m_importType, m_types)) });

		// -- Source Path and Output Path --
		contentBox->addSlot({ {0,0}, makePropertyRow("Source File", makeFileRow(m_sourcePath, "3D Models", "*.obj;*.gltf;*.glb", "meshes")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Output Location", makeDirectoryRow(m_outputPath, "meshes")) });
	}

	void MeshImportModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		std::string finalName = m_name + ".axmesh";
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

	void MeshImportModal::onConfirm() {
		std::string finalName = m_name + ".axmesh";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		UUID newAssetUUID = UUID::generate();

		AAP::MeshAssetData data;
		data.uuid = newAssetUUID;
		data.name = m_name;
		data.fileFormat = AAP::FormatUtils::meshFormatFromString(m_types[m_importType]);
		data.filePath = AssetManager::getRelativeToAssets(m_sourcePath);

		AAP::MeshParser::createTextFile(data, finalPath);

		AssetMetadata metadata;
		metadata.handle = newAssetUUID;
		metadata.type = AssetType::Mesh;
		metadata.filePath = AssetManager::getRelativeToAssets(finalPath);

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		registry->add(metadata);
		registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
	}

}
