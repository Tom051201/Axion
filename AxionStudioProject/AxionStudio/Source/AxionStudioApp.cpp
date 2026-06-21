#include <Axion.h>
#include <AxionEngine/Source/core/EntryPoint.h>

#include "EditorLayer.h"
#include "AxionStudio/Source/core/EditorAssetLoader.h"

namespace Axion {

	class AxionStudio : public Application {
	public:

		AxionStudio(const ApplicationSpecification& spec) : Application(spec) {

			pushLayer(new EditorLayer());
		}

		~AxionStudio() override {}

	};


	Application* createApplication() {
		WindowProperties windowProperties;
		windowProperties.title = "Axion Studio";
		windowProperties.dragAcceptFiles = true;
		windowProperties.iconFilePath = "AxionStudio/Resources/logo.ico";

		ApplicationSpecification spec;
		spec.windowProperties = windowProperties;
		spec.guiLayoutFilePath = "AxionStudio/Config/Layout.ini"; // TODO: check if still needed
		spec.guiSyleSetter = []() {}; // TODO: check if still needed
		spec.assetLoader = new EditorAssetLoader();

		return new AxionStudio(spec);
	}

}
