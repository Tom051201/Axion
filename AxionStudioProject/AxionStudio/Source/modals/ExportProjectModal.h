#pragma once
#include "AxionStudio/Source/core/Modal.h"

namespace Axion {

	class ExportProjectModal : public Modal {
	public:
		ExportProjectModal(const char* name);
		~ExportProjectModal() = default;

		void open() override;

	private:

		void renderContent() override;

		char m_exportPathBuffer[256] = "";
		bool m_openAfterExport = true;

	};

}