#pragma once

#include "AxionStudio/Source/core/Modal.h"

#include <string>

namespace Axion {

	class ExportProjectModal : public Modal {
	public:
		ExportProjectModal(const char* name) : Modal(name) {}
		~ExportProjectModal() override = default;

	private:

		void renderContent() override;

		std::string m_exportPath;
		bool m_openAfterExport = true;

	};

}