#include "MaterialImportModal.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionAssetPipeline/Source/parser/MaterialParser.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SSliderFloat.h"
#include "AxionStudio/Vendor/Silica/include/SColorPicker.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

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

	Silica::WidgetPtr MaterialImportModal::getWidget(Silica::FontAtlas* font, std::function<void()> onClose) {
		m_font = font;
		m_onClose = onClose;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(0, 0, 0, 180)
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void MaterialImportModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void MaterialImportModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 10.0f });

		// -- Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Create Material", .font = m_font}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 2}, .backgroundColor = Silica::Color(80, 80, 80, 255)}) });


		// -- Helper Functions --
		auto MakePropertyRow = [&](const std::string& label, Silica::WidgetPtr valueWidget) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(120.0f, 0.0f),
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

		auto MakeSliderRow = [&](float& val, float maxVal) {
			return Silica::MakeWidget<Silica::SSliderFloat>({
				.initialValue = val, .minValue = 0.0f, .maxValue = maxVal,
				.onValueChanged = [this, &val](float v) { val = v; }
			});
		};

		auto MakeFileRow = [&](std::string& outPath, const std::string& typeDesc, const std::string& filter) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {1,0}, Silica::MakeWidget<Silica::SBox>({
						.backgroundColor = Silica::Color(35, 35, 35, 255),
						.child = Silica::MakeWidget<Silica::SEditableText>({
							.initialText = outPath, .font = m_font,
							.onTextChanged = [this, &outPath](const std::string& val) { outPath = val; rebuildUI(); }
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = {8, 4}, .color = Silica::Color(50, 50, 50, 255),
						.onClick = [this, &outPath, typeDesc, filter]() {
							std::filesystem::path startDir = ProjectManager::getProject()->getAssetsPath();
							std::filesystem::path absPath = FileDialogs::openFile({ {typeDesc, filter} }, startDir);
							if (!absPath.empty()) { outPath = absPath.string(); rebuildUI(); }
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse...", .font = m_font})
					})}
				}
			});
		};

		auto MakeColorPicker = [&](Vec4& vecColor) {
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
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_name, .font = m_font,
				.onTextChanged = [this](const std::string& val) { m_name = val; rebuildUI(); }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Name", nameInput) });


		// -- Base Values --
		contentBox->addSlot({ {0,0}, MakePropertyRow("Albedo Color", MakeColorPicker(m_albedoColor)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Metalness", MakeSliderRow(m_metalness, 1.0f)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Roughness", MakeSliderRow(m_roughness, 1.0f)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Emission", MakeSliderRow(m_emission, 10.0f)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Tiling", MakeSliderRow(m_tiling, 100.0f)) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 1}, .backgroundColor = Silica::Color(60, 60, 60, 255)}) });


		// -- Texture Maps --
		contentBox->addSlot({ {0,0}, MakePropertyRow("Albedo Map", MakeFileRow(m_albedoMapPath, "Texture", "*.axtex")) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Normal Map", MakeFileRow(m_normalMapPath, "Texture", "*.axtex")) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Metalness Map", MakeFileRow(m_metalnessMapPath, "Texture", "*.axtex")) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Roughness Map", MakeFileRow(m_roughnessMapPath, "Texture", "*.axtex")) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Occlusion Map", MakeFileRow(m_occlusionMapPath, "Texture", "*.axtex")) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Emissive Map", MakeFileRow(m_emissiveMapPath, "Texture", "*.axtex")) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 1}, .backgroundColor = Silica::Color(60, 60, 60, 255)}) });


		// -- Dependencies --
		contentBox->addSlot({ {0,0}, MakePropertyRow("Pipeline", MakeFileRow(m_pipelinePath, "Pipeline", "*.axpso")) });

		auto outFolderRow = Silica::MakeWidget<Silica::SHorizontalBox>({
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
						std::filesystem::path startDir = ProjectManager::getProject()->getAssetsPath();
						std::filesystem::path absPath = FileDialogs::openFolder(startDir);
						if (!absPath.empty()) { m_outputPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse...", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Output Location", outFolderRow) });


		// -- Validation Logic --
		std::string finalName = m_name + ".axmat";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		auto checkFile = [](const std::string& path, bool& has, bool& exists, bool& isFile) {
			has = !path.empty();
			exists = true; isFile = true;
			if (has) { exists = std::filesystem::exists(path); isFile = std::filesystem::is_regular_file(path); }
		};

		bool hasPipe, pipeExists, pipeFile; checkFile(m_pipelinePath, hasPipe, pipeExists, pipeFile);
		bool hasAlbedo, albedoExists, albedoFile; checkFile(m_albedoMapPath, hasAlbedo, albedoExists, albedoFile);
		bool hasNormal, normalExists, normalFile; checkFile(m_normalMapPath, hasNormal, normalExists, normalFile);
		bool hasMetal, metalExists, metalFile; checkFile(m_metalnessMapPath, hasMetal, metalExists, metalFile);
		bool hasRough, roughExists, roughFile; checkFile(m_roughnessMapPath, hasRough, roughExists, roughFile);
		bool hasOcc, occExists, occFile; checkFile(m_occlusionMapPath, hasOcc, occExists, occFile);
		bool hasEmiss, emissExists, emissFile; checkFile(m_emissiveMapPath, hasEmiss, emissExists, emissFile);

		bool outputExists = std::filesystem::exists(m_outputPath);
		bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
		bool invalidOutFileName = std::filesystem::exists(finalPath);
		bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;

		bool disabled = (m_name.empty() || m_outputPath.empty() || !pipeExists || !pipeFile || !albedoExists || !albedoFile ||
			!normalExists || !normalFile || !metalExists || !metalFile || !roughExists || !roughFile || !occExists || !occFile ||
			!emissExists || !emissFile || !outputExists || !outputIsDirectory || invalidOutFileName || nameTooLong);

		std::string validationMsg = "Ready to create asset.";
		Silica::Color validationColor = Silica::Color(50, 255, 50, 255);

		if (disabled) {
			validationColor = Silica::Color(255, 50, 50, 255);
			if (m_name.empty()) validationMsg = "No name is set.";
			else if (m_outputPath.empty()) validationMsg = "No output directory is set.";
			else if (!pipeExists) validationMsg = "Pipeline file does not exist.";
			else if (!albedoExists || !normalExists || !metalExists || !roughExists || !occExists || !emissExists) validationMsg = "A texture map does not exist.";
			else if (!outputIsDirectory) validationMsg = "Output is not a directory.";
			else if (invalidOutFileName) validationMsg = "Asset with this name already exists.";
			else if (nameTooLong) validationMsg = "Name exceeds max limit.";
		}

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = validationMsg, .color = validationColor, .font = m_font}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 2}, .backgroundColor = Silica::Color(80, 80, 80, 255)}) });


		// -- Footer Buttons --
		auto createBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f },
			.color = disabled ? Silica::Color(60, 60, 60, 255) : Silica::Color(50, 150, 50, 255),
			.hoverColor = disabled ? Silica::Color::transparent() : Silica::Color(70, 180, 70, 255),
			.onClick = [this, disabled, finalPath, hasPipe, hasAlbedo, hasNormal, hasMetal, hasRough, hasOcc, hasEmiss]() {
				if (disabled) return Silica::EventReply::unhandled();

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
				prop.useNormalMap = 0.0f; prop.useMetalnessMap = 0.0f; prop.useRoughnessMap = 0.0f; prop.useOcclusionMap = 0.0f;

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

				if (m_onClose) m_onClose();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({
				.text = "Create",
				.color = disabled ? Silica::Color(120, 120, 120, 255) : Silica::Color::white(),
				.font = m_font,
			})
		});

		auto cancelBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f }, .color = Silica::Color(80, 80, 80, 255),
			.onClick = [this]() {
				if (m_onClose) m_onClose();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cancel", .font = m_font})
		});

		std::string versionText = "v" + std::to_string(ASSET_VERSION_MATERIAL);

		auto footerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, createBtn },
				{ {0,0}, cancelBtn },
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({.backgroundColor = Silica::Color::transparent()}) },
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = versionText, .color = Silica::Color(100,100,100,255), .font = m_font})
				})}
			}
		});

		contentBox->addSlot({ {0,0}, footerRow });


		// -- Assemble Modal --
		auto modalPanel = Silica::MakeWidget<Silica::SBox>({
			.padding = { 20.0f, 20.0f },
			.explicitSize = Silica::Vec2{ 600.0f, 0.0f },
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
