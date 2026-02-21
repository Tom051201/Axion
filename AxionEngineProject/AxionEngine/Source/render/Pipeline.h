#pragma once

#include "AxionEngine/Source/render/Shader.h"

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

	struct PipelineSpecification {
		Ref<Shader> shader;
		BufferLayout vertexLayout;

		ColorFormat colorFormat = ColorFormat::RGBA8;
		DepthStencilFormat depthStencilFormat = DepthStencilFormat::DEPTH32F;

		bool depthTest = true;
		bool depthWrite = true;
		DepthCompare depthFunction = DepthCompare::Less;

		bool stencilEnabled = false;
		uint32_t sampleCount = 1;

		CullMode cullMode = CullMode::Back;
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;

		uint32_t numRenderTargets = 1;
	};

	class Pipeline {
	public:

		virtual ~Pipeline() = default;

		virtual void release() = 0;

		virtual void bind() = 0;
		virtual void unbind() = 0;

		virtual const PipelineSpecification& getSpecification() const = 0;

		static Ref<Pipeline> create(const PipelineSpecification& spec);
	};

}
