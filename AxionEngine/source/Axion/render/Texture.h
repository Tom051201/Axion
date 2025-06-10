#pragma once

#include "Axion/core/Core.h"
#include <string>

namespace Axion {

	class Texture {
	public:

		virtual ~Texture() = default;

		virtual void bind(uint32_t slot) const = 0;

		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;

	};

	class Texture2D : public Texture {
	public:

		virtual ~Texture2D() = default;

		static Ref<Texture2D> create(const std::string& path);


	};

}

