#pragma once

#include "AxionStudio/Source/core/Modal.h"

namespace Axion {

	class PhysicsMaterialImportModal : public Modal {
	public:

		PhysicsMaterialImportModal(const char* name);
		~PhysicsMaterialImportModal();

		void close() override;

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[128] = "";
		char m_outputPathBuffer[256] = "";

		float m_staticFriction = 0.5f;
		float m_dynamicFriction = 0.5f;
		float m_restitution = 0.05f;

	};

}
