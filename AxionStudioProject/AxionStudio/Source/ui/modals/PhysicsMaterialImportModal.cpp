#include "PhysicsMaterialImportModal.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionAssetPipeline/Source/parser/PhysicsMaterialParser.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SSliderFloat.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	void PhysicsMaterialImportModal::resetInputs() {
		m_name.clear();

		std::filesystem::path phymatDir = ProjectManager::getProject()->getAssetsPath() / "physics";
		m_outputPath = phymatDir.string();

		m_staticFriction = 0.5f;
		m_dynamicFriction = 0.5f;
		m_restitution = 0.05f;
	}

	Silica::WidgetPtr PhysicsMaterialImportModal::getWidget(Silica::FontAtlas* font, std::function<void()> onClose) {
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

	void PhysicsMaterialImportModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			 m_rebuildQueued = false;
			 rebuildUI_Internal();
		});
	}

	void PhysicsMaterialImportModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 12.0f });

		// -- Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Import Physics Material", .font = m_font}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 2}, .backgroundColor = Silica::Color(80, 80, 80, 255)}) });


		// -- Helper Functions --
		auto MakePropertyRow = [&](const std::string& label, Silica::WidgetPtr valueWidget) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(130.0f, 0.0f),
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


		// -- Name --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_name, .font = m_font,
				.onTextChanged = [this](const std::string& val) { m_name = val; rebuildUI(); }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Name", nameInput) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 1}, .backgroundColor = Silica::Color(60, 60, 60, 255)}) });


		// -- Physics Properties --
		contentBox->addSlot({ {0,0}, MakePropertyRow("Static Friction", MakeSliderRow(m_staticFriction, 10.0f)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Dynamic Friction", MakeSliderRow(m_dynamicFriction, 10.0f)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Restitution", MakeSliderRow(m_restitution, 1.0f)) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 1}, .backgroundColor = Silica::Color(60, 60, 60, 255)}) });


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
						std::filesystem::path phymatDir = ProjectManager::getProject()->getAssetsPath() / "physics";
						std::filesystem::path absPath = std::filesystem::exists(phymatDir) ?
							FileDialogs::openFolder(phymatDir) : FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
						if (!absPath.empty()) { m_outputPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse...", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Output Location", outputRow) });


		// -- Validation Logic --
		std::string finalName = m_name + ".axpmat";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		bool outputExists = std::filesystem::exists(m_outputPath);
		bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
		bool invalidOutFileName = std::filesystem::exists(finalPath);
		bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;

		bool disabled = (m_name.empty() || m_outputPath.empty() || !outputExists || !outputIsDirectory || invalidOutFileName || nameTooLong);

		std::string validationMsg = "Ready to create asset.";
		Silica::Color validationColor = Silica::Color(50, 255, 50, 255);

		if (disabled) {
			validationColor = Silica::Color(255, 50, 50, 255);
			if (m_name.empty()) validationMsg = "Name needs to be set.";
			else if (m_outputPath.empty()) validationMsg = "No output directory is set.";
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

				AAP::PhysicsMaterialAssetData data;
				data.uuid = newAssetUUID;
				data.name = m_name;
				data.staticFriction = m_staticFriction;
				data.dynamicFriction = m_dynamicFriction;
				data.restitution = m_restitution;

				AAP::PhysicsMaterialParser::createTextFile(data, finalPath);

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::PhysicsMaterial;
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

		std::string versionText = "v" + std::to_string(ASSET_VERSION_PHYSICS_MATERIAL);

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
			.explicitSize = Silica::Vec2{ 500.0f, 0.0f },
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
