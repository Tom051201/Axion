#include "EditorCameraPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/render/RenderCommand.h"

namespace Axion {

	EditorCameraPanel::EditorCameraPanel(const std::string& name, EditorCamera2D* cam2D, EditorCamera3D* cam3D) : Panel(name) {
		m_camera2D = cam2D;
		m_camera3D = cam3D;
	}

	EditorCameraPanel::~EditorCameraPanel() {
		shutdown();
	}

	void EditorCameraPanel::setup() {
		
	}

	void EditorCameraPanel::shutdown() {

	}

	void EditorCameraPanel::onGuiRender() {
		if (ImGui::Begin("Editor Camera")) {

			ImGui::Text("Camera 3D");
			ImGui::InputFloat3("Position#3d", m_camera3D->getPosition().data());
			ImGui::DragFloat("Movement Speed", m_camera3D->getTranslationSpeedData(), 0.5f, 0.0f, 25.0f);

			// add fov

			ImGui::Separator();
			ImGui::Text("Camera 2D");
			ImGui::InputFloat2("Position#2d", m_camera2D->getPosition().data());
		}
		ImGui::End();

		if (ImGui::Begin("Renderer Stats")) {
			ImGui::Text(std::to_string(RenderCommand::getDrawCallCount()).c_str());
		}
		ImGui::End();
	}

}
