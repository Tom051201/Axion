#pragma once

#include "AxionStudio/Source/core/Panel.h"
#include "AxionStudio/Source/core/EditorCamera3D.h"

namespace Axion {

	class EditorCameraPanel : public Panel {
	public:

		EditorCameraPanel(const std::string& name, EditorCamera3D* cam);
		~EditorCameraPanel() override;

		void setup() override;
		void shutdown() override;
		void onGuiRender() override;

	private:

		EditorCamera3D* m_camera = nullptr;

	};

}
