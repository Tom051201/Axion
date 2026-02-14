#pragma once

#include "AxionStudio/Source/core/Panel.h"
#include "AxionStudio/Source/core/EditorCamera.h"

namespace Axion {

	class EditorCameraPanel : public Panel {
	public:

		EditorCameraPanel(const std::string& name, EditorCamera* cam);
		~EditorCameraPanel() override;

		void setup() override;
		void shutdown() override;
		void onGuiRender() override;

	private:

		EditorCamera* m_camera = nullptr;
	};

}
