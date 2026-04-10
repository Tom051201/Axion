#pragma once

#include "AxionEngine/Source/core/Version.h"

#include "AxionStudio/Source/core/Modal.h"

#include <string>

namespace Axion {

	class CreateProjectModal : public Modal {
	public:

		CreateProjectModal(const char* name) : Modal(name) {}
		~CreateProjectModal() override = default;

	private:

		void renderContent() override;
		void resetInputs() override;

		std::string m_name;
		std::string m_outputPath;
		std::string m_author;
		std::string m_company;
		std::string m_description;
		Version m_version = Version(1, 0, 0);

	};

}
