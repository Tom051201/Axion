#include "studiopch.h"
#include "MaterialImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SColorPicker.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SButton.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionAssetPipeline/Source/parser/MaterialParser.h"

namespace Axion {

	MaterialImportModal::MaterialImportModal() {
		m_modalTitle = "Create Material";
		m_versionText = "v" + std::to_string(ASSET_VERSION_MATERIAL);
		m_modalWidth = 600.0f;
		resetInputs();
	}

	void MaterialImportModal::resetInputs() {
		m_name.clear();
		m_pipelinePath.clear();

		std::filesystem::path matDir = ProjectManager::getProject()->getAssetsPath() / "materials";
		m_outputPath = matDir.string();

		m_albedoColor = Vec4::one();
		m_metalness = 0.0f;
		m_roughness = 0.0f;
		m_emission = 0.0f;
		m_tiling = 0.0f;
		m_albedoMapPath.clear();
		m_normalMapPath.clear();
		m_metalnessMapPath.clear();
		m_roughnessMapPath.clear();
		m_occlusionMapPath.clear();
		m_emissiveMapPath.clear();
	}

	void MaterialImportModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {

		auto makeColorPicker = [&](Vec4& vecColor) {
			Silica::Color initialColor = Silica::Color((uint8_t)(vecColor.x * 255), (uint8_t)(vecColor.y * 255), (uint8_t)(vecColor.z * 255), (uint8_t)(vecColor.w * 255));
			return Silica::MakeWidget<Silica::SMenuAnchor>({
				.openOnHover = false,
				.anchorContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { 2.0f, 2.0f }, .backgroundColor = Silica::Color(45, 45, 45, 255),
					.child = Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{100.0f, 24.0f}, .backgroundColor = initialColor })
				}),
				.menuContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 10.0f }, .backgroundColor = Silica::Color(45, 45, 45, 255),
					.child = Silica::MakeWidget<Silica::SColorPicker>({
						.initialColor = initialColor,
						.onColorChanged = [this, &vecColor](Silica::Color c) { vecColor = Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f); }
					})
				})
			});
			};

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

		// -- Base Values --
		contentBox->addSlot({ {0,0}, makePropertyRow("Albedo Color", makeColorPicker(m_albedoColor)) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Metalness", makeSliderRow(m_metalness, 1.0f)) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Roughness", makeSliderRow(m_roughness, 1.0f)) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Emission", makeSliderRow(m_emission, 10.0f)) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Tiling", makeSliderRow(m_tiling, 100.0f)) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.thickness = 2.0f }) });

		// -- Texture Maps --
		contentBox->addSlot({ {0,0}, makePropertyRow("Albedo Map", makeFileRow(m_albedoMapPath, "Axion Texture Asset", "*.axtex", "textures")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Normal Map", makeFileRow(m_normalMapPath, "Axion Texture Asset", "*.axtex", "textures")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Metalness Map", makeFileRow(m_metalnessMapPath, "Axion Texture Asset", "*.axtex", "textures")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Roughness Map", makeFileRow(m_roughnessMapPath, "Axion Texture Asset", "*.axtex", "textures")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Occlusion Map", makeFileRow(m_occlusionMapPath, "Axion Texture Asset", "*.axtex", "textures")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Emissive Map", makeFileRow(m_emissiveMapPath, "Axion Texture Asset", "*.axtex", "textures")) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.thickness = 2.0f }) });

		// -- Dependencies and Output --
		contentBox->addSlot({ {0,0}, makePropertyRow("Pipeline", makeFileRow(m_pipelinePath, "Axion Pipeline Asset", "*.axpso", "pipelines")) });
		contentBox->addSlot({ {0,0}, makePropertyRow("Output Location", makeDirectoryRow(m_outputPath, "materials")) });
	}

	void MaterialImportModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		std::string finalName = m_name + ".axmat";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		auto checkFile = [](const std::string& path, bool& has, bool& exists, bool& isFile) {
			has = !path.empty();
			exists = true;
			isFile = true;
			if (has) {
				std::error_code ec;
				exists = std::filesystem::exists(path, ec);
				isFile = std::filesystem::is_regular_file(path, ec);
			}
		};

		bool hasPipe, pipeExists, pipeFile; checkFile(m_pipelinePath, hasPipe, pipeExists, pipeFile);
		bool hasAlbedo, albedoExists, albedoFile; checkFile(m_albedoMapPath, hasAlbedo, albedoExists, albedoFile);
		bool hasNormal, normalExists, normalFile; checkFile(m_normalMapPath, hasNormal, normalExists, normalFile);
		bool hasMetal, metalExists, metalFile; checkFile(m_metalnessMapPath, hasMetal, metalExists, metalFile);
		bool hasRough, roughExists, roughFile; checkFile(m_roughnessMapPath, hasRough, roughExists, roughFile);
		bool hasOcc, occExists, occFile; checkFile(m_occlusionMapPath, hasOcc, occExists, occFile);
		bool hasEmiss, emissExists, emissFile; checkFile(m_emissiveMapPath, hasEmiss, emissExists, emissFile);

		bool outputExists = false;
		bool outputIsDirectory = false;
		bool invalidOutFileName = false;

		try {
			std::error_code ec;
			if (!m_outputPath.empty()) {
				outputExists = std::filesystem::exists(m_outputPath, ec);
				outputIsDirectory = std::filesystem::is_directory(m_outputPath, ec);
				invalidOutFileName = std::filesystem::exists(finalPath, ec);
			}
		}
		catch (...) {}

		bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;

		bool disabled = (m_name.empty() || m_outputPath.empty() || !pipeExists || !pipeFile || !albedoExists || !albedoFile ||
			!normalExists || !normalFile || !metalExists || !metalFile || !roughExists || !roughFile || !occExists || !occFile ||
			!emissExists || !emissFile || !outputExists || !outputIsDirectory || invalidOutFileName || nameTooLong);

		std::string validationMsg = "Ready to create asset.";
		Silica::Color validationColor = Silica::GetTheme().Text_Success;

		if (disabled) {
			validationColor = Silica::GetTheme().Text_Danger;
			if (m_name.empty()) validationMsg = "No name is set.";
			else if (m_outputPath.empty()) validationMsg = "No output directory is set.";
			else if (!pipeExists) validationMsg = "Pipeline file does not exist.";
			else if (!albedoExists || !normalExists || !metalExists || !roughExists || !occExists || !emissExists) validationMsg = "A texture map does not exist.";
			else if (!outputIsDirectory) validationMsg = "Output is not a directory.";
			else if (invalidOutFileName) validationMsg = "Asset with this name already exists.";
			else if (nameTooLong) validationMsg = "Name exceeds max limit.";
		}

		m_validationText->setText(validationMsg);
		m_validationText->setColor(validationColor);
		m_confirmBtn->setEnabled(!disabled);
	}

	void MaterialImportModal::onConfirm() {
		std::string finalName = m_name + ".axmat";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		bool hasPipe = !m_pipelinePath.empty();
		bool hasAlbedo = !m_albedoMapPath.empty();
		bool hasNormal = !m_normalMapPath.empty();
		bool hasMetal = !m_metalnessMapPath.empty();
		bool hasRough = !m_roughnessMapPath.empty();
		bool hasOcc = !m_occlusionMapPath.empty();
		bool hasEmiss = !m_emissiveMapPath.empty();

		UUID newAssetUUID = UUID::generate();
		AAP::MaterialAssetData data;
		data.uuid = newAssetUUID;
		data.name = m_name;
		if (hasPipe) data.pipelineAsset = AssetManager::getRelativeToAssets(m_pipelinePath);

		MaterialProperties prop;
		prop.albedoColor = m_albedoColor;
		prop.metalness = m_metalness;
		prop.roughness = m_roughness;
		prop.emissionStrength = m_emission;
		prop.tiling = m_tiling;
		prop.useNormalMap = 0.0f;
		prop.useMetalnessMap = 0.0f;
		prop.useRoughnessMap = 0.0f;
		prop.useOcclusionMap = 0.0f;

		if (hasAlbedo) data.textures[TextureSlot::Albedo] = AssetManager::getRelativeToAssets(m_albedoMapPath);
		if (hasNormal) { data.textures[TextureSlot::Normal] = AssetManager::getRelativeToAssets(m_normalMapPath); prop.useNormalMap = 1.0f; }
		if (hasMetal) { data.textures[TextureSlot::Metalness] = AssetManager::getRelativeToAssets(m_metalnessMapPath); prop.useMetalnessMap = 1.0f; }
		if (hasRough) { data.textures[TextureSlot::Roughness] = AssetManager::getRelativeToAssets(m_roughnessMapPath); prop.useRoughnessMap = 1.0f; }
		if (hasOcc) { data.textures[TextureSlot::Occlusion] = AssetManager::getRelativeToAssets(m_occlusionMapPath); prop.useOcclusionMap = 1.0f; }
		if (hasEmiss) { data.textures[TextureSlot::Emissive] = AssetManager::getRelativeToAssets(m_emissiveMapPath); prop.useEmissiveMap = 1.0f; }

		data.properties = prop;
		AAP::MaterialParser::createTextFile(data, finalPath);

		AssetMetadata metadata;
		metadata.handle = newAssetUUID;
		metadata.type = AssetType::Material;
		metadata.filePath = AssetManager::getRelativeToAssets(finalPath);

		auto registry = ProjectManager::getProject()->getAssetRegistry();
		registry->add(metadata);
		registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
	}

}
