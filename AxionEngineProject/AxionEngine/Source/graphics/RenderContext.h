#pragma once

namespace Axion {

	class RenderContext {
	public:

		virtual ~RenderContext() = default;

		virtual void* getNativeCommandList() const = 0;

		virtual void begin() = 0;
		virtual void end() = 0;

	};

}
