#pragma once

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"

namespace Axion {

	class SceneHierarchyPanel {
	public:

		SceneHierarchyPanel();
		~SceneHierarchyPanel();

		void setup(Ref<Scene> activeScene);
		void shutdown();

		void onGuiRender();

	private:

		Ref<Scene> m_activeScene;
		Entity m_selectedEntity;

		void displayEntity(Entity entity);


	};

}
