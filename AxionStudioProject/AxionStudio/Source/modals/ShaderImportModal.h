#pragma once

#include "AxionEngine/Source/render/Shader.h"

#include "AxionStudio/Source/core/Modal.h"

namespace Axion {

	class ShaderImportModal : public Modal {
	public:

		ShaderImportModal(const char* name);
		~ShaderImportModal() override;

		void close() override;

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[128] = "";
		char m_sourcePathBuffer[256] = "";
		char m_outputPathBuffer[256] = "";

		int m_formatIndex = 0;
		const char* m_formats[2] = { ".hlsl", ".glsl" };

		int m_colorFormatIndex = 1;
		ColorFormat m_colorFormats[6] = { ColorFormat::None, ColorFormat::RGBA8, ColorFormat::RED_INTEGER, ColorFormat::RGBA16F, ColorFormat::BGRA8, ColorFormat::RGB10A2 };
		const char* m_colorFormatsNames[6] = { "None", "RGBA8", "RED_INTEGER", "RGBA16F", "BGRA8", "RGB10A2" };

		int m_depthFormatIndex = 2;
		DepthStencilFormat m_depthFormats[5] = { DepthStencilFormat::None, DepthStencilFormat::DEPTH24_STENCIL8, DepthStencilFormat::DEPTH32F, DepthStencilFormat::DEPTH32F_STENCIL8, DepthStencilFormat::DEPTH16 };
		const char* m_depthFormatsNames[5] = { "None", "DEPTH24_STENCIL8", "DEPTH32F", "DEPTH32F_STENCIL8", "DEPTH16" };

		bool m_depthTest = true;
		bool m_depthWrite = true;

		int m_depthCompareIndex = 1;
		DepthCompare m_depthCompares[8] = { DepthCompare::Never, DepthCompare::Less, DepthCompare::Equal, DepthCompare::LessEqual, DepthCompare::Greater, DepthCompare::NotEqual, DepthCompare::GreaterEqual, DepthCompare::Always };
		const char* m_depthCompareNames[8] = { "Never", "Less", "Equal", "Less Equal", "Greater", "Not Equal", "Greater Equal", "Always" };

		bool m_stencilEnabled = false;
		int m_sampleCount = 1;

		int m_cullModeIndex = 2;
		CullMode m_cullModes[3] = { CullMode::None, CullMode::Front, CullMode::Back };
		const char* m_cullModesNames[3] = { "None", "Front", "Back" };

		int m_topologyIndex = 3;
		PrimitiveTopology m_topologies[5] = { PrimitiveTopology::PointList, PrimitiveTopology::LineList, PrimitiveTopology::LineStrip, PrimitiveTopology::TriangleList, PrimitiveTopology::TriangleStrip };
		const char* m_topologiesNames[5] = { "Point List", "Line List", "Line Strip", "Triangle List", "Triangle Strip" };

		int m_batchTexturesCount = 1;

		std::vector<BufferElement> m_bufferElements;
		const char* m_shaderDataTypeNames[10] = { "None", "Float", "Float2", "Float3", "Float4", "Int", "Int2", "Int3", "Int4", "Bool" };

	};

}
