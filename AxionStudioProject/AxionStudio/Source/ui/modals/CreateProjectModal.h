#pragma once

#include "AxionEngine/Source/core/Version.h"

#include "AxionStudio/Source/ui/ModalBase.h"

namespace Axion {

	class CreateProjectModal : public ModalBase {
	public:

		CreateProjectModal();

		Silica::WidgetPtr getWidget();

	protected:

		void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) override;
		void validate() override;
		void onConfirm() override;

	private:

		std::string m_name;
		std::string m_outputPath;
		std::string m_author;
		std::string m_company;
		std::string m_description;
		Version m_version = Version(1, 0, 0);

		void resetInputs();

	};

}
