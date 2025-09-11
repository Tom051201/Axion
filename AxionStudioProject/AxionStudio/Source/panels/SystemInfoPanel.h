#pragma once
#include "axpch.h"

#include "AxionStudio/Source/core/Panel.h"

namespace Axion {

	struct SystemInfo {
		std::string gpuName = "?";
		std::string gpuDriverVersion = "?";
		uint64_t vramMB = 0;

		std::string cpuName = "?";
		uint32_t cores = 0;

		uint64_t totalRamMB = 0;
		std::string os = "?";
	};

	class SystemInfoPanel : public Panel {
	public:

		SystemInfoPanel(const std::string& name);
		~SystemInfoPanel() override;

		void setup() override;
		void shutdown() override;
		void onGuiRender() override;

		SystemInfo& getSystemInfo() { return m_systemInfo; }
		const SystemInfo& getSystemInfo() const { return m_systemInfo; }

	private:

		SystemInfo m_systemInfo;

	};

}
