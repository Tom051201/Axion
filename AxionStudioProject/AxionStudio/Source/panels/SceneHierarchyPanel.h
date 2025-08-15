#pragma once

#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Entity.h"

namespace Axion {

	class SceneHierarchyPanel {
	public:

		SceneHierarchyPanel();
		~SceneHierarchyPanel();

		void setup(const Ref<Scene>& activeScene);
		void shutdown();

		void setContext(const Ref<Scene>& context);

		void onGuiRender();

	private:

		Ref<Scene> m_context = nullptr;
		Entity m_selectedEntity;

		void displayEntity(Entity entity);

	};

}
