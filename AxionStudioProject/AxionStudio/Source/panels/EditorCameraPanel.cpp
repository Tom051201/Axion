#include "EditorCameraPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/render/RenderCommand.h"

namespace Axion {

	EditorCameraPanel::EditorCameraPanel() {}

	EditorCameraPanel::~EditorCameraPanel() {
		shutdown();
	}

	void EditorCameraPanel::setup(EditorCamera3D* cam) {
		m_camera = cam;
	}

	void EditorCameraPanel::shutdown() {

	}

	void EditorCameraPanel::onGuiRender() {
		if (ImGui::Begin("Editor Camera")) {

			ImGui::InputFloat3("Position", m_camera->getPosition().data());

			ImGui::DragFloat("Movement Speed", m_camera->getTranslationSpeedData(), 0.5f, 0.0f, 25.0f);

			// add fov
		}
		ImGui::End();

		if (ImGui::Begin("Renderer Stats")) {
			ImGui::Text(std::to_string(RenderCommand::getDrawCallCount()).c_str());
		}
		ImGui::End();
	}

}
