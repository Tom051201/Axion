#include "EditorCameraPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/render/Renderer.h"

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
			ImGui::DragFloat("Rotation Speed#3d", &m_camera->m_rotationSpeed3D, 0.001f, 0.0f, 0.01f);
			ImGui::Text("Pitch: %f", m_camera->m_pitch);
			ImGui::Text("Yaw: %f", m_camera->m_yaw);
			ImGui::Text("Distance: %f", m_camera->m_distance);
			ImGui::Text("FOV: %f", m_camera->m_fov);
			
			ImGui::Separator();
			ImGui::Text("Orthographic Camera (2D)");
			ImGui::DragFloat("Movement Speed#2d", &m_camera->m_keyboardSpeed2D, 0.5f, 0.0f, 25.0f);
			ImGui::DragFloat("Drag Speed#2d", &m_camera->m_dragSpeed2D, 0.005f, 0.0f, 0.1f);
		}
		ImGui::End();

		if (ImGui::Begin("Renderer Stats")) {
			auto& stats = Renderer::getStats();

			ImGui::Text("Frame Time: %.2f ms", Renderer::getFrameTimeMs());
			ImGui::Text("Total Draw Calls: %d", stats.drawCalls);

			ImGui::Separator();
			ImGui::Text("--- 3D Renderer ---");
			ImGui::Text("Batches Drawn (Meshes): %d", stats.meshCount3D);
			ImGui::Text("Total 3D Instances: %d", stats.instanceCount3D);

			ImGui::Separator();
			ImGui::Text("--- 2D Renderer ---");
			ImGui::Text("Quads: %d", stats.quadCount2D);
			ImGui::Text("Vertices: %d", stats.getTotalVertexCount2D());
			ImGui::Text("Indices: %d", stats.getTotalIndexCount2D());
		}
		ImGui::End();
	}

}
