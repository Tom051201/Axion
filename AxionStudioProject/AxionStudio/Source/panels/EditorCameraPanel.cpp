#include "EditorCameraPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/render/RenderCommand.h"

namespace Axion {

	EditorCameraPanel::EditorCameraPanel(const std::string& name, EditorCamera* cam) : Panel(name) {
		m_camera = cam;
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

			ImGui::InputFloat3("Position#3d", m_camera->getPosition().data());

			ImGui::Separator();
			ImGui::Text("Perspective Camera (3D)");
			ImGui::DragFloat("Movement Speed#3d", &m_camera->m_translationSpeed3D, 0.5f, 0.0f, 25.0f);
			ImGui::DragFloat("Rotation Speed#3d", &m_camera->m_rotationSpeed3D, 0.001, 0.0f, 0.01f);
			ImGui::Text("Pitch: %f", m_camera->m_pitch);
			ImGui::Text("Yaw: %f", m_camera->m_yaw);
			ImGui::Text("Distance: %f", m_camera->m_distance);
			ImGui::Text("FOV: %f", m_camera->m_fov);
			
			ImGui::Separator();
			ImGui::Text("Orthographic Camera (2D)");
			ImGui::DragFloat("Movement Speed#2d", &m_camera->m_keyboardSpeed2D, 0.5f, 0.0f, 25.0f);
			ImGui::DragFloat("Drag Speed#2d", &m_camera->m_dragSpeed2D, 0.005, 0.0f, 0.1f);
		}
		ImGui::End();

		if (ImGui::Begin("Renderer Stats")) {
			ImGui::Text(std::to_string(RenderCommand::getDrawCallCount()).c_str());
		}
		ImGui::End();
	}

}
