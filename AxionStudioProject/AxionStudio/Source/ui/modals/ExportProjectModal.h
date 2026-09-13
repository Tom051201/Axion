#pragma once

#include "AxionStudio/Source/ui/ModalBase.h"

namespace Axion {

	class ExportProjectModal : public ModalBase {
	public:

		ExportProjectModal();

		Silica::WidgetPtr getWidget();

	protected:

		void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) override;
		void validate() override;
		void onConfirm() override;

	private:

		std::string m_exportPath;
		bool m_openAfterExport = true;

	};

}
