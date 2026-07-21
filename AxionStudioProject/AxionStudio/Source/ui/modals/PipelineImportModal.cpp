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
#include "AxionStudio/Vendor/Silica/include/SSeparator.h"
#include "AxionStudio/Vendor/Silica/include/Theme.h"

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

	Silica::WidgetPtr PipelineImportModal::getWidget(std::function<void()> onClose) {
		m_onClose = onClose;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.consumePointerEvents = true,
				.backgroundColor = Silica::Color(0, 0, 0, 180),
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
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Create Pipeline Asset" }) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });


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
					.child = Silica::MakeWidget<Silica::SScrollBox>({.child = menuBox})
				})
			});
		};

		// -- Core Properties --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_name ,
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
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = std::to_string(m_sampleCount) ,
				.onTextCommitted = [this](const std::string& val) {
					try { m_sampleCount = std::max(1, std::stoi(val)); }
					catch (...) {}
					rebuildUI();
				}
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Sample Count", countInput) });

		contentBox->addSlot({ {0,0}, MakePropertyRow("Cull Mode", MakeCombo(m_cullModeIndex, m_cullModesNames, 3)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Topology", MakeCombo(m_topologyIndex, m_topologiesNames, 5)) });

		auto rtInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = std::to_string(m_renderTargetsCount) ,
				.onTextCommitted = [this](const std::string& val) {
					try { m_renderTargetsCount = std::max(0, std::stoi(val)); }
					catch (...) {}
					rebuildUI();
				}
			})
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Render Targets", rtInput) });


		// -- Buffer Layout --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{0, 1},
			.backgroundColor = Silica::GetTheme().Background_Popup
		}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = "Buffer Layout:",
			.color = Silica::GetTheme().Background_Input
		}) });

		auto layoutBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 6.0f });

		for (size_t i = 0; i < m_bufferElements.size(); i++) {
			auto& element = m_bufferElements[i];

			auto elName = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{100, 0},
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = element.name ,
					.onTextChanged = [&element](const std::string& v) { element.name = v; }
				})
			});

			int tIdx = static_cast<int>(element.type);
			auto elType = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{90,0},
				.child = MakeCombo(tIdx, m_shaderDataTypeNames, 10)
			});

			if (static_cast<ShaderDataType>(tIdx) != element.type) {
				element.type = static_cast<ShaderDataType>(tIdx);
				element.size = ShaderDataTypeSize(element.type);
			}

			auto elNorm = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = element.normalized, .onCheckChanged = [&element](bool v) { element.normalized = v; } });
			auto elInst = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = element.instanced, .onCheckChanged = [&element](bool v) { element.instanced = v; } });

			auto delBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = {6,2},
				.color = Silica::GetTheme().Accent_Danger,
				.onClick = [this, i]() {
					m_bufferElements.erase(m_bufferElements.begin() + i);
					rebuildUI();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "X" })
			});

			layoutBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {1,0}, elName },
					{ {0,0}, elType },
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({.verticalAlign = Silica::VerticalAlign::Center, .child = Silica::MakeWidget<Silica::STextBlock>({.text = "Norm" })}) },
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({.verticalAlign = Silica::VerticalAlign::Center, .child = elNorm }) },
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({.verticalAlign = Silica::VerticalAlign::Center, .child = Silica::MakeWidget<Silica::STextBlock>({.text = "Inst" })}) },
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({.verticalAlign = Silica::VerticalAlign::Center, .child = elInst }) },
					{ {0,0}, delBtn }
				}
			}) });
		}

		auto addBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				m_bufferElements.emplace_back("Attribute", ShaderDataType::Float3);
				rebuildUI();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "+ Add Attribute" })
		});
		layoutBox->addSlot({ {0,0}, addBtn });

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({
			.padding = {10,10},
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = layoutBox
		}) });


		// -- File Paths --
		auto shaderRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_shaderPath ,
						.onTextChanged = [this](const std::string& val) { m_shaderPath = val; rebuildUI(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this]() {
						std::filesystem::path shaderDir = ProjectManager::getProject()->getAssetsPath() / "shaders";
						if (!std::filesystem::exists(shaderDir)) {
							shaderDir = ProjectManager::getProject()->getAssetsPath();
						}
						std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Shader Asset", "*.axshader"} }, shaderDir);
						if (!absPath.empty()) { m_shaderPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
				})}
			}
		});
		contentBox->addSlot({ {0,0}, MakePropertyRow("Shader File", shaderRow) });

		auto outputRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({
					.child = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_outputPath ,
						.onTextChanged = [this](const std::string& val) { m_outputPath = val; rebuildUI(); }
					})
				})},
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {8, 4},
					.onClick = [this]() {
						std::filesystem::path pipeDir = ProjectManager::getProject()->getAssetsPath() / "pipelines";
						if (!std::filesystem::exists(pipeDir)) {
							pipeDir = ProjectManager::getProject()->getAssetsPath();
						}
						std::filesystem::path absPath = FileDialogs::openFolder(pipeDir);
						if (!absPath.empty()) { m_outputPath = absPath.string(); rebuildUI(); }
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Browse..." })
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
		Silica::Color validationColor = Silica::GetTheme().Text_Success;

		if (disabled) {
			validationColor = Silica::GetTheme().Text_Danger;
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

		std::string versionText = "v" + std::to_string(ASSET_VERSION_PIPELINE);

		auto footerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0,0}, createBtn },
				{ {0,0}, cancelBtn },
				{ {1,0}, Silica::MakeWidget<Silica::SBox>({ .backgroundColor = Silica::Color::transparent()}) },
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

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SScrollBox>({
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = modalPanel
			})
		}));
	}

}
