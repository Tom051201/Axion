#include "AudioImportModal.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/audio/AudioManager.h"

#include "AxionAssetPipeline/Source/parser/AudioParser.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/Theme.h"
#include "AxionStudio/Vendor/Silica/include/SSeparator.h"

namespace Axion {

	void AudioImportModal::presetFromFile(const std::filesystem::path& sourceFile) {
		resetInputs();

		m_sourcePath = sourceFile.string();
		std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "audio";
		m_outputPath = audioDir.string();
		m_name = sourceFile.stem().string();

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

	Silica::WidgetPtr AudioImportModal::getWidget(std::function<void()> onClose) {
		m_onClose = onClose;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(0, 0, 0, 180),
//				.hoverColor = Silica::Color(0, 0, 0, 180)
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void AudioImportModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void AudioImportModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 12.0f });

		// -- Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Import Audio Asset" }) });
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
//					.hoverColor = Silica::GetTheme().Element_Hover,
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
				.initialText = m_name,
				.onTextChanged = [this](const std::string& val) { m_name = val; rebuildUI(); }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Name", nameInput) });


		// -- Format --
		contentBox->addSlot({ {0,0}, MakePropertyRow("Format", MakeCombo(m_importFormat, m_formatNames, 3)) });


		// -- Type --
		contentBox->addSlot({ {0,0}, MakePropertyRow("Type", MakeCombo(m_loadType, m_typesNames, 2)) });


		// -- Source Path --
		auto sourceRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_sourcePath,
						.onTextChanged = [this](const std::string& val) { m_sourcePath = val; rebuildUI(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this]() {
						std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "audio";
						std::filesystem::path absPath = std::filesystem::exists(audioDir) ?
							FileDialogs::openFile({ {"Audio Files", "*.mp3;*.wav;*.ogg"} }, audioDir) :
							FileDialogs::openFile({ {"Audio Files", "*.mp3;*.wav;*.ogg"} }, ProjectManager::getProject()->getAssetsPath());
						if (!absPath.empty()) { m_sourcePath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..."})
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
						.initialText = m_outputPath,
						.onTextChanged = [this](const std::string& val) { m_outputPath = val; rebuildUI(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this]() {
						std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "audio";
						std::filesystem::path absPath = std::filesystem::exists(audioDir) ?
							FileDialogs::openFolder(audioDir) : FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
						if (!absPath.empty()) { m_outputPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..."})
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Output Location", outputRow) });


		// -- Validation Logic --
		std::string finalName = m_name + ".axaudio";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		bool sourceExists = std::filesystem::exists(m_sourcePath);
		bool sourceIsFile = std::filesystem::is_regular_file(m_sourcePath);
		bool outputExists = std::filesystem::exists(m_outputPath);
		bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
		bool invalidOutFileName = std::filesystem::exists(finalPath);
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

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = validationMsg,
			.color = validationColor
		}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });


		// -- Footer Buttons --
		auto createBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f },
			.enabled = !disabled,
			.onClick = [this, disabled, finalPath]() {
				if (disabled) return Silica::EventReply::unhandled();

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

				if (m_onClose) m_onClose();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Create" })
		});

		auto cancelBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 20.0f, 8.0f },
			.onClick = [this]() {
				if (m_onClose) m_onClose();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Cancel" })
		});

		std::string versionText = "v" + std::to_string(ASSET_VERSION_AUDIO);

		auto footerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, createBtn },
				{ {0,0}, cancelBtn },
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({ .backgroundColor = Silica::Color::transparent() }) },
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
			.padding = { 20.0f, 20.0f },
			.explicitSize = Silica::Vec2{ 550.0f, 0.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.backgroundColor = Silica::GetTheme().Background_Panel,
//			.hoverColor = Silica::GetTheme().Background_Panel,
			.child = contentBox
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalPanel
		}));
	}

}
