#pragma once

#include "AxionStudio/Source/core/EditorCamera3D.h"

namespace Axion {

	class EditorCameraPanel {
	public:

		EditorCameraPanel();
		~EditorCameraPanel();

		void setup(EditorCamera3D* cam);
		void shutdown();

		void onGuiRender();

	private:

		EditorCamera3D* m_camera = nullptr;

	};

}
