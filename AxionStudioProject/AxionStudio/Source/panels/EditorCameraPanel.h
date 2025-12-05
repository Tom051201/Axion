#pragma once

#include "AxionStudio/Source/core/Panel.h"
#include "AxionStudio/Source/core/EditorCamera3D.h"
#include "AxionStudio/Source/core/EditorCamera2D.h"

namespace Axion {

	class EditorCameraPanel : public Panel {
	public:

		EditorCameraPanel(const std::string& name, EditorCamera2D* cam2D, EditorCamera3D* cam3D);
		~EditorCameraPanel() override;

		void setup() override;
		void shutdown() override;
		void onGuiRender() override;

	private:

		EditorCamera2D* m_camera2D = nullptr;
		EditorCamera3D* m_camera3D = nullptr;

	};

}
