#pragma once
#include "axpch.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// Shader ///////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class Shader {
	public:

		virtual ~Shader() = default;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual void release() = 0;

		virtual const std::string& getName() const = 0;

		virtual void compileFromFile(const std::string& filePath) = 0;

		static Ref<Shader> create(const std::string& name);
		static std::string readShaderFile(const std::string& filePath);
	};

	////////////////////////////////////////////////////////////////////////////////
	///// ShaderLibrary ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class ShaderLibrary {
	public:

		void release();

		void add(const Ref<Shader>& shader);
		Ref<Shader> load(const std::string& filePath);
		Ref<Shader> load(const std::string& name, const std::string& filePath);

		Ref<Shader> get(const std::string& name);

	private:

		std::unordered_map<std::string, Ref<Shader>> m_shaders;

		std::string getNameFromFile(const std::string& filePath);

	};

}
