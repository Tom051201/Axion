#include "studiopch.h"
#include "PipelineImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SCheckbox.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SInputFieldInt.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/SComboBox.h>
#include <Silica/include/SSpacer.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/PipelineParser.h"

#include "AxionStudio/Source/ui/SilicaHelpers.h"

namespace Axion {

	PipelineImportModal::PipelineImportModal() {
		m_modalTitle = "Create Pipeline Asset";
		m_versionText = "v" + std::to_string(ASSET_VERSION_PIPELINE);
		m_modalWidth = 600.0f;
		resetInputs();
	}

	void PipelineImportModal::resetInputs() {
		m_name.clear();
		m_shaderPath.clear();

		m_outputPath.clear();
		std::filesystem::path pipDir = ProjectManager::getProject()->getAssetsPath() / "Pipelines";
		if (std::filesystem::exists(pipDir)) m_outputPath = pipDir.generic_string();

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

	void PipelineImportModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {

		// -- Core Properties --
		auto nameInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_name,
				.onTextChanged = [this](const std::string& val) {
					m_name = val;
					validate();
				}
			})
		});

		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Name", nameInput) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSpacer>({.size = {0.0f, EditorTheme::SPACING_SMALL} }) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Color Format", makeCombo(m_colorFormatIndex, m_colorFormatsNames)) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Depth Stencil", makeCombo(m_depthFormatIndex, m_depthFormatsNames)) });

		auto depthTestCheck = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = m_depthTest, .onCheckChanged = [this](bool val) { m_depthTest = val; } });
		auto depthWriteCheck = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = m_depthWrite, .onCheckChanged = [this](bool val) { m_depthWrite = val; } });
		auto stencilCheck = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = m_stencilEnabled, .onCheckChanged = [this](bool val) { m_stencilEnabled = val; } });

		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Depth Test", depthTestCheck) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Depth Write", depthWriteCheck) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Depth Compare", makeCombo(m_depthCompareIndex, m_depthCompareNames)) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Stencil", stencilCheck) });

		auto countInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SInputFieldInt>({
				.initialValue = m_sampleCount,
				.onValueChanged = [this](int val) { m_sampleCount = std::max(1, val); }
			})
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Sample Count", countInput) });

		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Cull Mode", makeCombo(m_cullModeIndex, m_cullModesNames)) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Topology", makeCombo(m_topologyIndex, m_topologiesNames)) });

		auto rtInput = Silica::MakeWidget<Silica::SBox>({
			.child = Silica::MakeWidget<Silica::SInputFieldInt>({
				.initialValue = m_renderTargetsCount,
				.onValueChanged = [this](int val) { m_renderTargetsCount = std::max(0, val); }
			})
		});
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Render Targets", rtInput) });

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.space = EditorTheme::SPACING_LARGE}) });

		// -- Buffer Layout --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = "Buffer Layout:",
			.color = Silica::GetTheme().Text_Main
		}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSpacer>({.size = {0.0f, EditorTheme::SPACING_SMALL} }) });

		auto layoutBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_MEDIUM });

		for (size_t i = 0; i < m_bufferElements.size(); i++) {
			auto& element = m_bufferElements[i];

			auto elName = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{100, 0},
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = element.name,
					.onTextChanged = [&element](const std::string& v) { element.name = v; }
				})
			});

			int tIdx = static_cast<int>(element.type);
			auto elType = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{90,0},
				.child = makeCombo(tIdx, m_shaderDataTypeNames)
			});

			if (static_cast<ShaderDataType>(tIdx) != element.type) {
				element.type = static_cast<ShaderDataType>(tIdx);
				element.size = ShaderDataTypeSize(element.type);
			}

			auto elNorm = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = element.normalized, .onCheckChanged = [&element](bool v) { element.normalized = v; } });
			auto elInst = Silica::MakeWidget<Silica::SCheckBox>({ .initialCheck = element.instanced, .onCheckChanged = [&element](bool v) { element.instanced = v; } });

			auto delBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::PADDING_MEDIUM, EditorTheme::PADDING_SMALL },
				.color = Silica::GetTheme().Accent_Danger,
				.onClick = [this, i]() {
					m_bufferElements.erase(m_bufferElements.begin() + i);
					rebuildUI();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X" })
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
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "+ Add Attribute" })
		});
		layoutBox->addSlot({ {0,0}, addBtn });

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({
			.padding = {10,10},
			.explicitSize = Silica::Vec2{0.0f, 160.0f},
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SScrollBox>({
				.child = layoutBox
			})
		}) });
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.space = EditorTheme::SPACING_LARGE}) });


		// -- File Paths --
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Shader File", makeFileRow(m_shaderPath, "Axion Shader Asset", "*.axshader", "shaders")) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Output Location", makeDirectoryRow(m_outputPath, "pipelines")) });
	}

	void PipelineImportModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		std::string finalName = m_name + ".axpso";
		std::filesystem::path finalPath;

		bool shaderExists = false;
		bool shaderIsFile = false;
		bool outputExists = false;
		bool outputIsDirectory = false;
		bool invalidOutFileName = false;

		try {
			std::error_code ec;
			if (!m_shaderPath.empty()) {
				shaderExists = std::filesystem::exists(m_shaderPath, ec);
				shaderIsFile = std::filesystem::is_regular_file(m_shaderPath, ec);
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

		m_validationText->setText(validationMsg);
		m_validationText->setColor(validationColor);
		m_confirmBtn->setEnabled(!disabled);
	}

	void PipelineImportModal::onConfirm() {
		std::string finalName = m_name + ".axpso";
		std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

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
	}

}
