#include "studiopch.h"
#include "AxionEngine/Source/core/EntryPoint.h"

#include "AxionEngine/Source/core/Application.h"
#include "AxionEngine/Source/core/Window.h"
#include "AxionEngine/Source/layers/LayerStack.h"

#include "AxionStudio/Source/EditorLayer.h"
#include "AxionStudio/Source/core/EditorAssetLoader.h"


namespace Axion {

	class AxionStudio : public Application {
	public:

		AxionStudio(const ApplicationSpecification& spec) : Application(spec) {

			pushLayer(new EditorLayer());
		}

		~AxionStudio() override {}

	};


	Application* createApplication(ApplicationCommandLineArgs args) {
		WindowProperties windowProperties;
		windowProperties.title = "Axion Studio";
		windowProperties.dragAcceptFiles = true;
		windowProperties.iconFilePath = "AxionStudio/Resources/logo.ico";

		ApplicationSpecification spec;
		spec.windowProperties = windowProperties;
		spec.assetLoader = new EditorAssetLoader();
		spec.commandLineArgs = args;

		return new AxionStudio(spec);
	}

}
