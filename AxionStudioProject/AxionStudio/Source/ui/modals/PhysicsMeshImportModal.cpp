#include "studiopch.h"
#include "PhysicsMeshImportModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SButton.h>
#include <Silica/include/SSpacer.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/SInputFieldInt.h>

#include "AxionEngine/Source/EngineConfig.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/PhysicsMeshParser.h"
#include "AxionAssetPipeline/Source/importer/PhysicsCooker.h"

#include "AxionStudio/Source/ui/SilicaHelpers.h"

namespace Axion {

	PhysicsMeshImportModal::PhysicsMeshImportModal() {
		m_modalTitle = "Cook Physics Mesh";
		m_versionText = "v" + std::to_string(ASSET_VERSION_PHYSICS_MESH);
		m_modalWidth = 550.0f;
		resetInputs();
	}

	void PhysicsMeshImportModal::presetFromFile(const std::filesystem::path& sourceVisualMesh) {
		resetInputs();
		m_sourcePath = sourceVisualMesh.generic_string();

		std::string rawName = sourceVisualMesh.stem().string();
		if (!rawName.empty()) {
			rawName[0] = std::toupper(static_cast<unsigned char>(rawName[0]));
			std::replace(rawName.begin(), rawName.end(), ' ', '_');
		}
		m_name = rawName + "_Collider";
	}

	void PhysicsMeshImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();
		m_outputPath.clear();

		std::filesystem::path physDir = ProjectManager::getProject()->getAssetsPath() / "Physics";
		if (std::filesystem::exists(physDir)) m_outputPath = physDir.generic_string();
		else m_outputPath = ProjectManager::getProject()->getAssetsPath().generic_string();

		m_importType = 0;
		m_vertexLimit = 255;
	}

	void PhysicsMeshImportModal::buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) {
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
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Type", makeCombo(m_importType, m_types)) });

		if (m_importType == 0) {
			auto limitInput = Silica::MakeWidget<Silica::SInputFieldInt>({
				.initialValue = m_vertexLimit,
				.onValueChanged = [this](int val) { m_vertexLimit = std::clamp(val, 8, 255); }
			});
			contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Vertex Limit", limitInput) });
		}

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.space = EditorTheme::SPACING_LARGE}) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Source Mesh", makeFileRow(m_sourcePath, "Axion Mesh", "*.axmesh", "meshes")) });
		contentBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Output Location", makeDirectoryRow(m_outputPath, "physics")) });
	}

	void PhysicsMeshImportModal::validate() {
		if (!m_validationText || !m_confirmBtn) return;

		std::filesystem::path finalPath;
		bool sourceExists = false, outputExists = false, outputIsDirectory = false, invalidOutFileName = false;

		try {
			std::error_code ec;
			if (!m_sourcePath.empty()) sourceExists = std::filesystem::exists(m_sourcePath, ec);
			if (!m_outputPath.empty()) {
				outputExists = std::filesystem::exists(m_outputPath, ec);
				outputIsDirectory = std::filesystem::is_directory(m_outputPath, ec);
				finalPath = std::filesystem::path(m_outputPath) / (m_name + ".axpmesh");
				invalidOutFileName = std::filesystem::exists(finalPath, ec);
			}
		}
		catch (...) {}

		bool disabled = (m_name.empty() || m_sourcePath.empty() || m_outputPath.empty() || !sourceExists || !outputExists || !outputIsDirectory || invalidOutFileName);

		if (disabled) {
			m_validationText->setColor(Silica::GetTheme().Text_Danger);
			m_validationText->setText(invalidOutFileName ? "Asset with this name already exists." : "Invalid configuration.");
		}
		else {
			m_validationText->setColor(Silica::GetTheme().Text_Success);
			m_validationText->setText("Ready to cook.");
		}
		m_confirmBtn->setEnabled(!disabled);
	}

	void PhysicsMeshImportModal::onConfirm() {
		std::filesystem::path finalYamlPath = std::filesystem::path(m_outputPath) / (m_name + ".axpmesh");
		std::filesystem::path finalBinPath = std::filesystem::path(m_outputPath) / (m_name + ".axpgeom");

		UUID sourceUUID = AssetManager::getAssetUUID(m_sourcePath);
		AssetHandle<Mesh> handle = AssetManager::load<Mesh>(sourceUUID);
		Ref<Mesh> visualMesh = AssetManager::get<Mesh>(handle);

		if (!visualMesh) {
			AX_CORE_LOG_ERROR("Failed to load visual mesh for cooking.");
			return;
		}

		bool cookSuccess = false;
		if (m_importType == 0) cookSuccess = AAP::PhysicsCooker::cookConvex(visualMesh, m_vertexLimit, finalBinPath);
		else cookSuccess = AAP::PhysicsCooker::cookTriangle(visualMesh, finalBinPath);

		if (cookSuccess) {
			UUID newAssetUUID = UUID::generate();
			AAP::PhysicsMeshAssetData data;
			data.uuid = newAssetUUID;
			data.name = m_name;
			data.type = (m_importType == 0) ? PhysicsMesh::Type::Convex : PhysicsMesh::Type::Triangle;
			data.sourcePath = AssetManager::getRelativeToAssets(finalBinPath);

			AAP::PhysicsMeshParser::createTextFile(data, finalYamlPath);

			AssetMetadata metadata;
			metadata.handle = newAssetUUID;
			metadata.type = AssetType::PhysicsMesh;
			metadata.filePath = AssetManager::getRelativeToAssets(finalYamlPath);

			auto registry = ProjectManager::getProject()->getAssetRegistry();
			registry->add(metadata);
			registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
		}
	}

}
