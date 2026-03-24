#include <Axion.h>
#include <AxionEngine/Source/core/EntryPoint.h>

#include "RuntimeLayer.h"

namespace Axion {

	class RuntimeApplication : public Application {
	public:

		RuntimeApplication(const ApplicationSpecification& spec) : Application(spec) {
			pushLayer(new RuntimeLayer());
		}

		~RuntimeApplication() override {};

	};

	Application* createApplication() {

		WindowProperties windowProperties;
		windowProperties.title = "Game Name"; // TODO: read all from config file
		windowProperties.width = 1920;
		windowProperties.height = 1080; 
		windowProperties.dragAcceptFiles = false;

		ApplicationSpecification appSpec;
		appSpec.windowProperties = windowProperties;

		return new RuntimeApplication(appSpec);
	}

}
