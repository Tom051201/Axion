#include "axpch.h"
#include "GraphicsContext.h"

namespace Axion {

	static GraphicsContext* s_instance = nullptr;

	GraphicsContext* GraphicsContext::get() {
		return s_instance;
	}

	void GraphicsContext::set(GraphicsContext* context) {
		if (s_instance) {
			s_instance->shutdown();
			delete s_instance;
		}
		s_instance = context;
	}

}
