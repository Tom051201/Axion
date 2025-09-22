#include "SceneOverviewPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

// TODO: TEMP
#include "AxionAssetPipeline/Source/AxShader.h"
#include "AxionAssetPipeline/Source/AxMaterial.h"

namespace Axion {

	SceneOverviewPanel::SceneOverviewPanel(const std::string& name) : Panel(name) {}

	SceneOverviewPanel::~SceneOverviewPanel() {
		shutdown();
	}

	void SceneOverviewPanel::setup() {}

	void SceneOverviewPanel::shutdown() {}

	void SceneOverviewPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(SceneOverviewPanel::onSceneChanged));
	}

	void SceneOverviewPanel::onGuiRender() {
		ImGui::Begin("Scene Overview");

		if (!ProjectManager::hasProject()) {
			ImGui::TextWrapped("No Project Loaded. \nPlease load or create a project first.");
			ImGui::End();
			return;
		}

		if (!m_activeScene) {
			ImGui::TextWrapped("No Scene loaded");
			ImGui::End();
			return;
		}

		// -- Title --
		ImGui::TextUnformatted("Title");
		ImGui::SameLine();
		strcpy_s(m_titleBuffer, sizeof(m_titleBuffer), m_activeScene->getTitle().c_str());
		m_titleBuffer[sizeof(m_titleBuffer) - 1] = '\0';
		if (ImGui::InputText("##sceneTitle", m_titleBuffer, sizeof(m_titleBuffer))) {
			m_activeScene->setTitle(m_titleBuffer);
		}

		ImGui::SeparatorText("Skybox");
		if (m_activeScene->hasSkybox()) {
			// -- Has a skybox --
			std::filesystem::path skyPath = std::filesystem::path(m_activeScene->getSkyboxPath());
			std::filesystem::path skyRel = std::filesystem::relative(skyPath, ProjectManager::getProject()->getAssetsPath());
			ImGui::Text("Title: %s", skyPath.stem().string().c_str());
			ImGui::Text("Path: %s", skyRel.string().c_str());
		}
		else {
			// -- Does not have a skybox --
			ImGui::Text("No Skybox has been selected");
		}


		if (ImGui::Button("Select Skybox")) {
			std::string absolutePath = FileDialogs::openFile({ {"Axion Skybox Asset", "*.axsky"} }, ProjectManager::getProject()->getAssetsPath() + "\\skybox");
			if (!absolutePath.empty()) {
				AssetHandle<Skybox> handle = AssetManager::load<Skybox>(absolutePath);
				m_activeScene->setSkybox(handle);
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Remove")) {
			m_activeScene->removeSkybox();
		}

		// TODO: TEMP
		if (ImGui::Button("Create skybox shader")) {
			AAP::ShaderAssetData data;
			data.fileFormat = "HLSL";
			data.filePath = "shaders/skyboxShader.hlsl";
			ShaderSpecification spec{};
			spec.name = "SkyboxShader";
			spec.colorFormat = ColorFormat::RGBA8;
			spec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
			spec.depthTest = true;
			spec.depthWrite = false;
			spec.depthFunction = DepthCompare::LessEqual;
			spec.cullMode = CullMode::Back;
			spec.topology = PrimitiveTopology::TriangleList;
			spec.vertexLayout = {
				{ "POSITION", ShaderDataType::Float3 }
			};
			data.spec = spec;
			AAP::ShaderParser::createAxShaderFile(data, AssetManager::getAbsolute("shaders/skyboxShader.axshader"));
		}
		if (ImGui::Button("Create position shader")) {
			AAP::ShaderAssetData data;
			data.fileFormat = "HLSL";
			data.filePath = "shaders/positionShader.hlsl";
			ShaderSpecification spec{};
			spec.name = "PositionShader";
			spec.vertexLayout = {
				{ "POSITION", Axion::ShaderDataType::Float3 },
				{ "NORMAL", Axion::ShaderDataType::Float3 },
				{ "TEXCOORD", Axion::ShaderDataType::Float2 }
			};
			data.spec = spec;
			AAP::ShaderParser::createAxShaderFile(data, AssetManager::getAbsolute("shaders/positionShader.axshader"));
		}

		if (ImGui::Button("Create Basic Material")) {
			AAP::MaterialAssetData data;
			data.name = "BasicMaterial";
			data.color = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
			data.shaderAsset = "shaders/positionShader.axshader";
			AAP::MaterialParser::createAxMatFile(data, AssetManager::getAbsolute("materials/basicMaterial.axmat"));
		}

		ImGui::End();
	}

	bool SceneOverviewPanel::onSceneChanged(SceneChangedEvent& e) {

		setScene(SceneManager::getScene());

		return false;
	}

}
