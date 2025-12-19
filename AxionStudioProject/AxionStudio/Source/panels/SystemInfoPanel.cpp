#include "SystemInfoPanel.h"

#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

namespace Axion {

	SystemInfoPanel::SystemInfoPanel(const std::string& name) : Panel(name) {}

	SystemInfoPanel::~SystemInfoPanel() {
		shutdown();
	}

	void SystemInfoPanel::setup() {
		m_systemInfo.gpuName = GraphicsContext::get()->getGpuName();
		m_systemInfo.gpuDriverVersion = GraphicsContext::get()->getGpuDriverVersion();
		m_systemInfo.vramMB = GraphicsContext::get()->getVramMB();

		m_systemInfo.cpuName = PlatformInfo::getCpuName();
		m_systemInfo.cores = PlatformInfo::getCpuCores();

		m_systemInfo.totalRamMB = PlatformInfo::getRamMB();
		m_systemInfo.os = PlatformInfo::getOsVersion();
	}
	
	void SystemInfoPanel::shutdown() {}

	void SystemInfoPanel::onGuiRender() {
		if (ImGui::Begin("System Info")) {
			const auto& info = m_systemInfo;

			// ----- GPU -----
			ImGui::SeparatorText("GPU");
			ImGui::Columns(2, nullptr, false);

			ImGui::Text("GPU:");		ImGui::NextColumn();	ImGui::Text("%s", info.gpuName.c_str());			ImGui::NextColumn();
			ImGui::Text("VRAM:");		ImGui::NextColumn();	ImGui::Text("%llu MB", info.vramMB);				ImGui::NextColumn();
			ImGui::Text("Driver:");		ImGui::NextColumn();	ImGui::Text("%s", info.gpuDriverVersion.c_str());	ImGui::NextColumn();

			// ----- CPU -----
			ImGui::Columns(1);
			ImGui::SeparatorText("CPU");
			ImGui::Columns(2, nullptr, false);

			ImGui::Text("CPU:");		ImGui::NextColumn();	ImGui::Text("%s", info.cpuName.c_str());			ImGui::NextColumn();
			ImGui::Text("Cores:");		ImGui::NextColumn();	ImGui::Text("%u", info.cores);						ImGui::NextColumn();
			ImGui::Text("RAM:");		ImGui::NextColumn();	ImGui::Text("%llu MB", info.totalRamMB);			ImGui::NextColumn();
			ImGui::Text("Frame Time:");	ImGui::NextColumn();	ImGui::Text("%s ms", std::to_string(Renderer::getFrameTimeMs()).c_str());	ImGui::NextColumn();
			ImGui::Text("FPS:");		ImGui::NextColumn();	ImGui::Text("%s FPS", std::to_string(1000.0 / Renderer::getFrameTimeMs()).c_str());	ImGui::NextColumn();

			// ----- OS -----
			ImGui::Columns(1);
			ImGui::SeparatorText("Operating System");
			ImGui::Columns(2, nullptr, false);

			ImGui::Text("OS:");			ImGui::NextColumn();	ImGui::TextWrapped("%s", info.os.c_str());			ImGui::NextColumn();

			ImGui::Columns(1);
		}
		ImGui::End();
	}

}
