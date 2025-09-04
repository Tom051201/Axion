#pragma once
#include "axpch.h"

namespace Axion {

	class Texture2D {
	public:

		virtual ~Texture2D() = default;

		virtual void release() = 0;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;
		
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;

		virtual void* getHandle() const = 0;


		static Ref<Texture2D> create(const std::string& path);

	};

	class TextureCube {
	public:

		virtual ~TextureCube() = default;

		virtual void release() = 0;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void* getHandle() const = 0;

		static Ref<TextureCube> create(const std::array<std::string, 6>& filePaths);

	};

}
