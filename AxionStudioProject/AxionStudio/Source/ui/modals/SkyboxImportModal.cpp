#include "studiopch.h"
#include "SkyboxImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SSeparator.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/SkyboxParser.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	void SkyboxImportModal::resetInputs() {
		m_name.clear();
		m_texturePath.clear();
		m_pipelinePath.clear();

		std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "skybox";
		m_outputPath = dir.string();
	}

	void SkyboxImportModal::validate() {
		if (!m_validationText || !m_createBtn) return;

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
		m_createBtn->setEnabled(!disabled);
	}

	Silica::WidgetPtr SkyboxImportModal::getWidget(std::function<void()> onClose) {
		m_onClose = onClose;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.consumePointerEvents = true,
				.backgroundColor = Silica::Color(0, 0, 0, 180)
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void SkyboxImportModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void SkyboxImportModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 12.0f });

		// -- Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Import Skybox Asset" }) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });

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
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = label })
						})
					})},
					{ {1, 0}, valueWidget }
				}
			});
		};


		// -- Name --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_name ,
				.onTextChanged = [this](const std::string& val) { m_name = val; validate(); }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Name", nameInput) });


		// -- Texture Cube Path --
		auto textureRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_texturePath ,
						.onTextChanged = [this](const std::string& val) { m_texturePath = val; validate(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this]() {
						std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "textures";
						if (!std::filesystem::exists(dir)) {
							dir = ProjectManager::getProject()->getAssetsPath();
						}
						std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Texture File", "*.axtcube"} }, dir);
						if (!absPath.empty()) { m_texturePath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Texture Cube", textureRow) });


		// -- Pipeline Path --
		auto pipelineRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_pipelinePath ,
						.onTextChanged = [this](const std::string& val) { m_pipelinePath = val; validate(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this]() {
						std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "pipelines";
						if (!std::filesystem::exists(dir)) {
							dir = ProjectManager::getProject()->getAssetsPath();
						}
						std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Pipeline Asset", "*.axpso"} }, dir);
						if (!absPath.empty()) { m_pipelinePath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Pipeline", pipelineRow) });


		// -- Output Path --
		auto outputRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_outputPath ,
						.onTextChanged = [this](const std::string& val) { m_outputPath = val; validate(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this]() {
						std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "skybox";
						if (!std::filesystem::exists(dir)) {
							dir = ProjectManager::getProject()->getAssetsPath();
						}
						std::filesystem::path absPath = FileDialogs::openFolder(dir);
						if (!absPath.empty()) { m_outputPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Output Location", outputRow) });


		// -- Initialize Dynamic UI References --
		m_validationText = Silica::MakeWidget<Silica::STextBlock>({ .text = "" });

		m_createBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f },
			.onClick = [this]() {
				if (!m_createBtn->isEnabled()) return Silica::EventReply::unhandled();

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

				if (m_onClose) m_onClose();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Create" })
		});

		contentBox->addSlot({ {0,0}, m_validationText });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });


		// -- Footer Buttons --
		auto cancelBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f },
			.onClick = [this]() {
				if (m_onClose) m_onClose();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cancel" })
		});

		std::string versionText = "v" + std::to_string(ASSET_VERSION_SKYBOX);

		auto footerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, m_createBtn },
				{ {0,0}, cancelBtn },
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({.backgroundColor = Silica::Color::transparent()}) },
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({
						.text = versionText,
						.color = Silica::GetTheme().Text_Dim
					})
				})}
			}
		});

		contentBox->addSlot({ {0,0}, footerRow });


		// -- Assemble Modal --
		auto modalPanel = Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{ 550.0f, 0.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.backgroundColor = Silica::GetTheme().Background_Panel,
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = { 20.0f, 20.0f },
				.backgroundColor = Silica::Color::transparent(),
				.child = contentBox
			})
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalPanel
		}));

		validate();
	}

}
