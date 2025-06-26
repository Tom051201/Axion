#include "axpch.h"
#include "SystemInfoPanel.h"

#include "Axion/render/GraphicsContext.h"
#include "Axion/core/PlatformInfo.h"

#include "imgui.h"

namespace Axion {

	SystemInfoPanel::SystemInfoPanel() {}

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
		if (ImGui::Begin("System Info")/*, &m_showSystemInfoWindow*/) {
			const auto& info = m_systemInfo;

			ImGui::Columns(2, nullptr, false);

			ImGui::Text("GPU:");		ImGui::NextColumn();	ImGui::Text("%s", info.gpuName.c_str());			ImGui::NextColumn();
			ImGui::Text("VRAM:");		ImGui::NextColumn();	ImGui::Text("%llu MB", info.vramMB);				ImGui::NextColumn();
			ImGui::Text("Driver:");		ImGui::NextColumn();	ImGui::Text("%s", info.gpuDriverVersion.c_str());	ImGui::NextColumn();

			ImGui::Separator();			ImGui::Columns(2, nullptr, false);

			ImGui::Text("CPU:");		ImGui::NextColumn();	ImGui::Text("%s", info.cpuName.c_str());			ImGui::NextColumn();
			ImGui::Text("Cores:");		ImGui::NextColumn();	ImGui::Text("%u", info.cores);						ImGui::NextColumn();
			ImGui::Text("RAM:");		ImGui::NextColumn();	ImGui::Text("%llu MB", info.totalRamMB);			ImGui::NextColumn();

			ImGui::Separator();			ImGui::Columns(2, nullptr, false);

			ImGui::Text("OS:");			ImGui::NextColumn();	ImGui::TextWrapped("%s", info.os.c_str());			ImGui::NextColumn();

			ImGui::Columns(1);
		}
		ImGui::End();
	}


}
