#include <Axion.h>
#include <Axion/core/EntryPoint.h>

#include "EditorLayer.h"

namespace Axion {

	class AxionStudio : public Application {
	public:

		AxionStudio() {

			pushLayer(new EditorLayer());
		}

		~AxionStudio() override {}

	};


	Application* createApplication() {
		return new AxionStudio();
	}

}
