#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include <Silica/include/SWidget.h>

namespace Silica {
	class SBox;
}

namespace Axion {

	class SystemInfoModal {
	public:

		SystemInfoModal();
		~SystemInfoModal() = default;

		Silica::WidgetPtr getWidget();

	private:

		void rebuildUI();
		void rebuildUI_Internal();

		// -- System Info Data --
		std::string m_gpuName;
		std::string m_gpuDriverVersion;
		uint64_t m_vramMB = 0;

		std::string m_cpuName;
		uint32_t m_cores = 0;
		uint64_t m_totalRamMB = 0;

		std::string m_os;

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

	};

}
