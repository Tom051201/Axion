#include "Texture2DImportModal.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionAssetPipeline/Source/parser/Texture2DParser.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	void Texture2DImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();

		std::filesystem::path tex2dDir = ProjectManager::getProject()->getAssetsPath() / "textures";
		m_outputPath = tex2dDir.string();

		m_importType = 0;
	}

	Silica::WidgetPtr Texture2DImportModal::getWidget(Silica::FontAtlas* font, std::function<void()> onClose) {
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
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Import Texture2D Asset", .font = m_font}) });
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

		auto MakeCombo = [&](int& currentIndex, const char** names, int count) {
			auto menuBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });
			for (int i = 0; i < count; ++i) {
				menuBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.color = Silica::Color::transparent(),
					.hoverColor = Silica::Color(70, 130, 200, 255),
					.onClick = [this, &currentIndex, i]() {
						currentIndex = i;
						rebuildUI();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = names[i], .font = m_font})
				}) });
			}

			return Silica::MakeWidget<Silica::SMenuAnchor>({
				.openOnHover = false,
				.anchorContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { 8.0f, 4.0f },
					.backgroundColor = Silica::Color(45, 45, 45, 255),
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = names[currentIndex], .font = m_font})
				}),
				.menuContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { 4.0f, 4.0f },
					.backgroundColor = Silica::Color(35, 35, 35, 255),
					.child = menuBox
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


		// -- Type --
		contentBox->addSlot({ {0,0}, MakePropertyRow("Type", MakeCombo(m_importType, m_types, 3)) });


		// -- Source Path --
		auto sourceRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.backgroundColor = Silica::Color(35, 35, 35, 255),
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_sourcePath, .font = m_font,
						.onTextChanged = [this](const std::string& val) { m_sourcePath = val; rebuildUI(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4}, .color = Silica::Color(50, 50, 50, 255),
					.onClick = [this]() {
						std::filesystem::path tex2dDir = ProjectManager::getProject()->getAssetsPath() / "textures";
						std::filesystem::path absPath = std::filesystem::exists(tex2dDir) ?
							FileDialogs::openFile({ {"Image File", "*.png;*.jpg;*.jpeg"} }, tex2dDir) :
							FileDialogs::openFile({ {"Image File", "*.png;*.jpg;*.jpeg"} }, ProjectManager::getProject()->getAssetsPath());
						if (!absPath.empty()) { m_sourcePath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse...", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Source File", sourceRow) });


		// -- Output Path --
		auto outputRow = Silica::MakeWidget<Silica::SHorizontalBox>({
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
						std::filesystem::path tex2dDir = ProjectManager::getProject()->getAssetsPath() / "textures";
						std::filesystem::path absPath = std::filesystem::exists(tex2dDir) ?
							FileDialogs::openFolder(tex2dDir) : FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
						if (!absPath.empty()) { m_outputPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse...", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Output Location", outputRow) });


		// -- Validation Logic --
		std::string finalName = m_name + ".axtex";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		bool sourceExists = std::filesystem::exists(m_sourcePath);
		bool sourceIsFile = std::filesystem::is_regular_file(m_sourcePath);
		bool outputExists = std::filesystem::exists(m_outputPath);
		bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
		bool invalidOutFileName = std::filesystem::exists(finalPath);
		bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;

		bool disabled = (m_name.empty() || m_sourcePath.empty() || m_outputPath.empty() || !sourceExists || !sourceIsFile || !outputExists || !outputIsDirectory || invalidOutFileName || nameTooLong);

		std::string validationMsg = "Ready to create asset.";
		Silica::Color validationColor = Silica::Color(50, 255, 50, 255);

		if (disabled) {
			validationColor = Silica::Color(255, 50, 50, 255);
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

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = validationMsg, .color = validationColor, .font = m_font}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 2}, .backgroundColor = Silica::Color(80, 80, 80, 255)}) });


		// -- Footer Buttons --
		auto createBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f },
			.color = disabled ? Silica::Color(60, 60, 60, 255) : Silica::Color(50, 150, 50, 255),
			.hoverColor = disabled ? Silica::Color::transparent() : Silica::Color(70, 180, 70, 255),
			.onClick = [this, disabled, finalPath]() {
				if (disabled) return Silica::EventReply::unhandled();

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

		std::string versionText = "v" + std::to_string(ASSET_VERSION_TEXTURE2D);

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
			.explicitSize = Silica::Vec2{ 550.0f, 0.0f },
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
