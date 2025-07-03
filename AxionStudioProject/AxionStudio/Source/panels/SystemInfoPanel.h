#pragma once

#include "axpch.h"

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

	class SystemInfoPanel {
	public:

		SystemInfoPanel();
		~SystemInfoPanel();

		void setup();
		void shutdown();

		void onGuiRender();

		SystemInfo& getSystemInfo() { return m_systemInfo; }
		const SystemInfo& getSystemInfo() const { return m_systemInfo; }

	private:

		SystemInfo m_systemInfo;

	};

}
