#include "studiopch.h"
#include "Texture2DImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SSeparator.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/Texture2DParser.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	void Texture2DImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();

		std::filesystem::path tex2dDir = ProjectManager::getProject()->getAssetsPath() / "textures";
		m_outputPath = tex2dDir.string();

		m_importType = 0;
	}

	void Texture2DImportModal::validate() {
		if (!m_validationText || !m_createBtn) return;

		std::string finalName = m_name + ".axtex";
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
		m_createBtn->setEnabled(!disabled);
	}

	Silica::WidgetPtr Texture2DImportModal::getWidget(std::function<void()> onClose) {
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

	void Texture2DImportModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void Texture2DImportModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 12.0f });


		// -- Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Import Texture2D Asset" }) });
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

		auto MakeCombo = [&](int& currentIndex, const char** names, int count) {
			auto menuBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });
			for (int i = 0; i < count; ++i) {
				menuBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.color = Silica::Color::transparent(),
					.onClick = [this, &currentIndex, i]() {
						currentIndex = i;
						rebuildUI();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = names[i] })
				}) });
			}

			return Silica::MakeWidget<Silica::SMenuAnchor>({
				.openOnHover = false,
				.anchorContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { 8.0f, 4.0f },
					.backgroundColor = Silica::GetTheme().Element_Normal,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = names[currentIndex] })
				}),
				.menuContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { 4.0f, 4.0f },
					.backgroundColor = Silica::GetTheme().Background_Popup,
					.child = menuBox
				})
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


		// -- Type --
		contentBox->addSlot({ {0,0}, MakePropertyRow("Type", MakeCombo(m_importType, m_types, 3)) });


		// -- Source Path --
		auto sourceRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_sourcePath ,
						.onTextChanged = [this](const std::string& val) { m_sourcePath = val; validate(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this]() {
						std::filesystem::path tex2dDir = ProjectManager::getProject()->getAssetsPath() / "textures";
						if (!std::filesystem::exists(tex2dDir)) {
							tex2dDir = ProjectManager::getProject()->getAssetsPath();
						}
						std::filesystem::path absPath = FileDialogs::openFile({ {"Image File", "*.png;*.jpg;*.jpeg"} }, tex2dDir);
						if (!absPath.empty()) { m_sourcePath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Source File", sourceRow) });


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
						std::filesystem::path tex2dDir = ProjectManager::getProject()->getAssetsPath() / "textures";
						if (!std::filesystem::exists(tex2dDir)) {
							tex2dDir = ProjectManager::getProject()->getAssetsPath();
						}
						std::filesystem::path absPath = FileDialogs::openFolder(tex2dDir);
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

				std::string finalName = m_name + ".axtex";
				std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

				UUID newAssetUUID = UUID::generate();

				AAP::Texture2DAssetData data;
				data.uuid = newAssetUUID;
				data.name = m_name;
				data.fileFormat = AAP::FormatUtils::textureFormatFromString(m_types[m_importType]);
				data.filePath = AssetManager::getRelativeToAssets(m_sourcePath);

				AAP::Texture2DParser::createTextFile(data, finalPath);

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::Texture2D;
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

		std::string versionText = "v" + std::to_string(ASSET_VERSION_TEXTURE2D);

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
