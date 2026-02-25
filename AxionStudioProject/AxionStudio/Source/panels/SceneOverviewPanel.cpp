#include "SceneOverviewPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

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
		ImGui::SeparatorText("Scene Overview");

		// -- No project or scene loaded --
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


		if (ImGui::BeginTable("SceneOverviewTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Title --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Title");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			strcpy_s(m_titleBuffer, sizeof(m_titleBuffer), m_activeScene->getTitle().c_str());
			m_titleBuffer[sizeof(m_titleBuffer) - 1] = '\0';
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::InputText("##SceneTitle_input", m_titleBuffer, sizeof(m_titleBuffer))) {
				m_activeScene->setTitle(m_titleBuffer);
			}

			if (m_activeScene->hasSkybox()) {
				// -- Has a skybox --
				std::filesystem::path skyPath = AssetManager::getAssetFilePath<Skybox>(m_activeScene->getSkyboxHandle());
				std::filesystem::path skyRel = AssetManager::getRelativeToAssets(skyPath);

				// -- Skybox name --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Skybox");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(skyPath.stem().string().c_str());

				// -- Skybox file --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("File");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(skyRel.string().c_str());

				// -- Load skybox button --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Options");
				ImGui::TableSetColumnIndex(1);
				if (ImGui::Button("Select Skybox")) {
					std::filesystem::path skyDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "skybox";
					std::string absolutePath = FileDialogs::openFile({ {"Axion Skybox Asset", "*.axsky"} }, skyDir.string());
					if (!absolutePath.empty()) {
						AssetHandle<Skybox> handle = AssetManager::load<Skybox>(absolutePath);
						m_activeScene->setSkybox(handle);
					}
				}

				// -- Remove skybox button --
				ImGui::SameLine();
				if (ImGui::Button("Remove")) {
					m_activeScene->removeSkybox();
				}

			}
			else {
				// -- Does not have a skybox --

				// -- Skybox name --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Skybox");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("No Skybox Loaded");

				// -- Skybox file --
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("File");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("No Skybox Loaded");
			}

			// -- GlobalGravity --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Global Gravity");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::DragFloat3("##GlobalGravDrag", m_activeScene->getGravity().data(), 0.0f);

			ImGui::EndTable();
		}

		ImGui::End();
	}

	bool SceneOverviewPanel::onSceneChanged(SceneChangedEvent& e) {

		setScene(SceneManager::getScene());

		return false;
	}

}
