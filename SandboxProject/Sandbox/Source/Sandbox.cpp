#include "Axion.h"
#include "AxionEngine/Source/core/EntryPoint.h"

#include "Sandbox3D.h"

class Sandbox : public Axion::Application {
public:

	Sandbox() {
		pushLayer(new Sandbox3D());
	}
	~Sandbox() override {}

};


Axion::Application* Axion::createApplication() {
	return new Sandbox();
}
