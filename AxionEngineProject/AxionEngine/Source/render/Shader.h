#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Formats.h"

namespace Axion {

	enum class CullMode {
		None = 0,
		Front,
		Back
	};

	enum class DepthCompare {
		Never = 0,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always
	};

	enum class PrimitiveTopology {
		PointList = 0,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip
	};

	struct ShaderSpecification {
		std::string name;

		ColorFormat colorFormat = ColorFormat::RGBA8;
		DepthStencilFormat depthStencilFormat = DepthStencilFormat::DEPTH32F;

		bool depthTest = true;
		bool depthWrite = true;
		DepthCompare depthFunction = DepthCompare::Less;

		bool stencilEnabled = false;
		uint32_t sampleCount = 1;

		CullMode cullMode = CullMode::Back;
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;
	};

	////////////////////////////////////////////////////////////////////////////////
	///// Shader ///////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class Shader {
	public:

		virtual ~Shader() = default;

		virtual void release() = 0;

		virtual void bind() const = 0;
		virtual void unbind() const = 0;

		virtual const std::string& getName() const = 0;

		virtual void compileFromFile(const std::string& filePath) = 0;


		static Ref<Shader> create(const ShaderSpecification& spec);
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
