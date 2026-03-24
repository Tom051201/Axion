#pragma once

#include "AxionStudio/Source/core/Modal.h"

namespace Axion {

	class CreateProjectModal : public Modal {
	public:

		CreateProjectModal(const char* name);
		~CreateProjectModal();

		void close() override;

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[256] = "";
		char m_locationBuffer[256] = "";
		char m_authorBuffer[256] = "";
		char m_companyBuffer[256] = "";
		char m_descriptionBuffer[256] = "";

	};

}
