#include "PipelineImportModal.h"

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionAssetPipeline/Source/parser/PipelineParser.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SCheckbox.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	void PipelineImportModal::resetInputs() {
		m_name.clear();
		m_shaderPath.clear();

		std::filesystem::path pipeDir = ProjectManager::getProject()->getAssetsPath() / "pipelines";
		m_outputPath = pipeDir.string();

		m_colorFormatIndex = 1;
		m_depthFormatIndex = 2;
		m_depthTest = true;
		m_depthWrite = true;
		m_depthCompareIndex = 1;
		m_stencilEnabled = false;
		m_sampleCount = 1;
		m_cullModeIndex = 2;
		m_topologyIndex = 3;
		m_renderTargetsCount = 1;
		m_bufferElements.clear();
	}

	Silica::WidgetPtr PipelineImportModal::getWidget(Silica::FontAtlas* font, std::function<void()> onClose) {
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

	void PipelineImportModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void PipelineImportModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 10.0f });


		// -- Header --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Create Pipeline Asset", .font = m_font}) });
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
					.child = Silica::MakeWidget<Silica::SScrollBox>({.child = menuBox})
				})
			});
		};

		// -- Core Properties --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_name, .font = m_font,
				.onTextChanged = [this](const std::string& val) { m_name = val; rebuildUI(); }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Name", nameInput) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Color Format", MakeCombo(m_colorFormatIndex, m_colorFormatsNames, 6)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Depth Stencil", MakeCombo(m_depthFormatIndex, m_depthFormatsNames, 5)) });

		auto depthTestCheck = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = m_depthTest, .onCheckChanged = [this](bool val) { m_depthTest = val; } });
		auto depthWriteCheck = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = m_depthWrite, .onCheckChanged = [this](bool val) { m_depthWrite = val; } });
		auto stencilCheck = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = m_stencilEnabled, .onCheckChanged = [this](bool val) { m_stencilEnabled = val; } });

		contentBox->addSlot({ {0,0}, MakePropertyRow("Depth Test", depthTestCheck) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Depth Write", depthWriteCheck) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Depth Compare", MakeCombo(m_depthCompareIndex, m_depthCompareNames, 8)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Stencil", stencilCheck) });

		auto countInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = std::to_string(m_sampleCount), .font = m_font,
				.onTextCommitted = [this](const std::string& val) { try { m_sampleCount = std::max(1, std::stoi(val)); } catch (...) {} rebuildUI(); }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Sample Count", countInput) });

		contentBox->addSlot({ {0,0}, MakePropertyRow("Cull Mode", MakeCombo(m_cullModeIndex, m_cullModesNames, 3)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Topology", MakeCombo(m_topologyIndex, m_topologiesNames, 5)) });

		auto rtInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = std::to_string(m_renderTargetsCount), .font = m_font,
				.onTextCommitted = [this](const std::string& val) { try { m_renderTargetsCount = std::max(0, std::stoi(val)); } catch (...) {} rebuildUI(); }
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Render Targets", rtInput) });


		// -- Buffer Layout --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 1}, .backgroundColor = Silica::Color(60, 60, 60, 255)}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Buffer Layout:", .color = Silica::Color(180, 180, 180, 255), .font = m_font}) });

		auto layoutBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 6.0f });

		for (size_t i = 0; i < m_bufferElements.size(); i++) {
			auto& element = m_bufferElements[i];

			auto elName = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{100, 0}, .backgroundColor = Silica::Color(35, 35, 35, 255),
				.child = Silica::MakeWidget<Silica::SEditableText>({.initialText = element.name, .font = m_font, .onTextChanged = [&element](const std::string& v) { element.name = v; } })
			});

			int tIdx = static_cast<int>(element.type);
			auto elType = Silica::MakeWidget<Silica::SBox>({ .explicitSize = Silica::Vec2{90,0}, .child = MakeCombo(tIdx, m_shaderDataTypeNames, 10) });

			if (static_cast<ShaderDataType>(tIdx) != element.type) {
				element.type = static_cast<ShaderDataType>(tIdx);
				element.size = ShaderDataTypeSize(element.type);
			}

			auto elNorm = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = element.normalized, .onCheckChanged = [&element](bool v) { element.normalized = v; } });
			auto elInst = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = element.instanced, .onCheckChanged = [&element](bool v) { element.instanced = v; } });

			auto delBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = {6,2}, .color = Silica::Color(150, 50, 50, 255),
				.onClick = [this, i]() { m_bufferElements.erase(m_bufferElements.begin() + i); rebuildUI(); return Silica::EventReply::handled(); },
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X", .font = m_font})
			});

			layoutBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {1,0}, elName },
					{ {0,0}, elType },
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({.verticalAlign = Silica::VerticalAlign::Center, .child = Silica::MakeWidget<Silica::STextBlock>({.text = "Norm",.font = m_font})}) },
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({.verticalAlign = Silica::VerticalAlign::Center, .child = elNorm }) },
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({.verticalAlign = Silica::VerticalAlign::Center, .child = Silica::MakeWidget<Silica::STextBlock>({.text = "Inst",.font = m_font})}) },
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({.verticalAlign = Silica::VerticalAlign::Center, .child = elInst }) },
					{ {0,0}, delBtn }
				}
			}) });
		}

		auto addBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4}, .color = Silica::Color(40, 100, 150, 255),
			.onClick = [this]() { m_bufferElements.emplace_back("Attribute", ShaderDataType::Float3); rebuildUI(); return Silica::EventReply::handled(); },
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "+ Add Attribute", .font = m_font})
		});
		layoutBox->addSlot({ {0,0}, addBtn });

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({
			.padding = {10,10}, .backgroundColor = Silica::Color(25, 25, 25, 255),
			.child = layoutBox
		}) });


		// -- File Paths --
		auto shaderRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.backgroundColor = Silica::Color(35, 35, 35, 255),
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_shaderPath, .font = m_font,
						.onTextChanged = [this](const std::string& val) { m_shaderPath = val; rebuildUI(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4}, .color = Silica::Color(50, 50, 50, 255),
					.onClick = [this]() {
						std::filesystem::path shaderDir = ProjectManager::getProject()->getAssetsPath() / "shaders";
						std::filesystem::path absPath = std::filesystem::exists(shaderDir) ?
							FileDialogs::openFile({ {"Shader Asset", "*.axshader"} }, shaderDir) :
							FileDialogs::openFile({ {"Shader Asset", "*.axshader"} }, ProjectManager::getProject()->getAssetsPath());
						if (!absPath.empty()) { m_shaderPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse...", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Shader File", shaderRow) });

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
						std::filesystem::path pipeDir = ProjectManager::getProject()->getAssetsPath() / "pipelines";
						std::filesystem::path absPath = std::filesystem::exists(pipeDir) ?
							FileDialogs::openFolder(pipeDir) : FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
						if (!absPath.empty()) { m_outputPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse...", .font = m_font})
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Output Location", outputRow) });


		// -- Validation Logic --
		std::string finalName = m_name + ".axpso";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

		bool shaderExists = std::filesystem::exists(m_shaderPath);
		bool shaderIsFile = std::filesystem::is_regular_file(m_shaderPath);
		bool outputExists = std::filesystem::exists(m_outputPath);
		bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
		bool invalidOutFileName = std::filesystem::exists(finalPath);
		bool nameTooLong = m_name.length() > Config::MaxBinaryStringLength;

		bool disabled = (m_name.empty() || m_shaderPath.empty() || m_outputPath.empty() || !shaderExists || !shaderIsFile || !outputExists || !outputIsDirectory || invalidOutFileName || nameTooLong);

		std::string validationMsg = "Ready to create asset.";
		Silica::Color validationColor = Silica::Color(50, 255, 50, 255);

		if (disabled) {
			validationColor = Silica::Color(255, 50, 50, 255);
			if (m_name.empty()) validationMsg = "No Name is set.";
			else if (m_shaderPath.empty()) validationMsg = "No shader file is set.";
			else if (m_outputPath.empty()) validationMsg = "No output directory is set.";
			else if (!shaderExists) validationMsg = "Shader file does not exist.";
			else if (!shaderIsFile) validationMsg = "Shader is not a file.";
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

				PipelineSpecification spec = {};
				spec.colorFormat = m_colorFormats[m_colorFormatIndex];
				spec.depthStencilFormat = m_depthFormats[m_depthFormatIndex];
				spec.depthTest = m_depthTest;
				spec.depthWrite = m_depthWrite;
				spec.depthFunction = m_depthCompares[m_depthCompareIndex];
				spec.stencilEnabled = m_stencilEnabled;
				spec.sampleCount = m_sampleCount;
				spec.cullMode = m_cullModes[m_cullModeIndex];
				spec.topology = m_topologies[m_topologyIndex];
				spec.numRenderTargets = m_renderTargetsCount;
				spec.vertexLayout = BufferLayout(m_bufferElements);

				UUID newAssetUUID = UUID::generate();
				AAP::PipelineAssetData data;
				data.uuid = newAssetUUID;
				data.shaderFilePath = AssetManager::getRelativeToAssets(m_shaderPath);
				data.name = m_name;
				data.spec = spec;

				AAP::PipelineParser::createTextFile(data, finalPath);

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::Pipeline;
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

		std::string versionText = "v" + std::to_string(ASSET_VERSION_PIPELINE);

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

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SScrollBox>({
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = modalPanel
			})
		}));
	}

}
