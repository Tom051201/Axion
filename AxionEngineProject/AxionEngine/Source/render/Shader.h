#pragma once
#include "axpch.h"

#include "AxionEngine/Source/render/Formats.h"
#include "AxionEngine/Source/render/Buffers.h"

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
		std::string name = "Unset";

		ColorFormat colorFormat = ColorFormat::RGBA8;
		DepthStencilFormat depthStencilFormat = DepthStencilFormat::DEPTH32F;

		bool depthTest = true;
		bool depthWrite = true;
		DepthCompare depthFunction = DepthCompare::Less;

		bool stencilEnabled = false;
		uint32_t sampleCount = 1;

		CullMode cullMode = CullMode::Back;
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;

		uint32_t batchTextures = 1;
		uint32_t numRenderTargets = 1;

		BufferLayout vertexLayout;
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
		virtual void recompile() = 0;


		static Ref<Shader> create(const ShaderSpecification& spec);
		static Ref<Shader> create(const ShaderSpecification& spec, const std::string& filePath);
		static std::string readShaderFile(const std::string& filePath);
	};

}
