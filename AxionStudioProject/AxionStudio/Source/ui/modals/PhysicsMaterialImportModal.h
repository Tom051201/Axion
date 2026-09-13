#pragma once

#include "AxionStudio/Source/ui/ModalBase.h"

namespace Axion {

	class PhysicsMaterialImportModal : public ModalBase {
	public:

		PhysicsMaterialImportModal();

	protected:

		void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) override;
		void validate() override;
		void onConfirm() override;

	private:

		std::string m_name;
		std::string m_outputPath;
		float m_staticFriction = 0.5f;
		float m_dynamicFriction = 0.5f;
		float m_restitution = 0.05f;

		void resetInputs();

	};

}
