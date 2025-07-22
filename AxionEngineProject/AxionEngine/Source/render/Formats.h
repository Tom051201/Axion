#pragma once

namespace Axion {

	enum class ColorFormat {
		None = 0,
		RGBA8,
		RED_INTEGER,
		RGBA16F,
		BGRA8,
		RGB10A2
	};

	enum class DepthStencilFormat {
		None = 0,
		DEPTH24_STENCIL8,
		DEPTH32F,
		DEPTH32F_STENCIL8,
		DEPTH16
	};

}
