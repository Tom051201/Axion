#include "Modal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

namespace Axion {

	void Modal::onGuiRender() {
		if (m_open) {
			ImGui::OpenPopup(m_name);

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(
				ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.5f),
				ImGuiCond_Appearing,
				ImVec2(0.5f, 0.5f)
			);
		}

		ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;
		if (!m_showTitle) { flags |= ImGuiWindowFlags_NoTitleBar; }
		if (ImGui::BeginPopupModal(m_name, NULL, flags)) {

			// -- Render specific content --
			renderContent();

			// -- Close Modal on ESC --
			if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				close();
			}

			ImGui::EndPopup();
		}
	}

	void Modal::open() {
		m_open = true;
	}

	void Modal::close() {
		m_open = false;
		ImGui::CloseCurrentPopup();
	}

}
