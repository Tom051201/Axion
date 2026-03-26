#pragma once

#include "AxionStudio/Source/core/Modal.h"

#include <string>

namespace Axion {

	class PhysicsMaterialImportModal : public Modal {
	public:

		PhysicsMaterialImportModal(const char* name) : Modal(name) {}
		~PhysicsMaterialImportModal() override = default;

	private:

		void renderContent() override;
		void resetInputs() override;

		std::string m_name;
		std::string m_outputPath;

		float m_staticFriction = 0.5f;
		float m_dynamicFriction = 0.5f;
		float m_restitution = 0.05f;

	};

}
