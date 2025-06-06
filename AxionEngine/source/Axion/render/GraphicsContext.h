#pragma once

#include "axpch.h"

namespace Axion {

	class GraphicsContext {
	public:

		virtual ~GraphicsContext() = default;

		virtual void initialize(void* windowHandle, uint32_t width, uint32_t height) = 0;
		virtual void shutdown() = 0;

		virtual void* getNativeContext() const = 0;

		static GraphicsContext* get();
		static void set(GraphicsContext* context);
	};

	

}
