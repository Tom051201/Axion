#include "Axion.h"
#include "AxionEngine/Source/core/EntryPoint.h"

#include "Sandbox2D.h"
#include "FrameBufferExample.h"

class Sandbox : public Axion::Application {
public:

	Sandbox() {
		pushLayer(new Sandbox2D());
		//pushLayer(new FrameBufferExample());
	}
	~Sandbox() override {}

};


Axion::Application* Axion::createApplication() {
	return new Sandbox();
}
