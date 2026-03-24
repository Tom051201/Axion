#pragma once
#include <Axion.h>

namespace Axion {

	class RuntimeLayer : public Layer {
	public:

		RuntimeLayer();
		~RuntimeLayer();

		void onAttach() override;
		void onDetach() override;

		void onUpdate(Timestep ts) override;
		void onEvent(Event& e) override;

	private:

		Ref<Scene> m_activeScene;

		bool onWindowResize(WindowResizeEvent& e);

	};

}
