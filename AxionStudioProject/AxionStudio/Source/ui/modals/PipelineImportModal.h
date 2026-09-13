#pragma once

#include <string>
#include <vector>
#include <array>

#include "AxionEngine/Source/graphics/Pipeline.h"

#include "AxionStudio/Source/ui/ModalBase.h"

namespace Axion {

	class PipelineImportModal : public ModalBase {
	public:

		PipelineImportModal();

	protected:

		void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) override;
		void validate() override;
		void onConfirm() override;

	private:

		std::string m_name;
		std::string m_shaderPath;
		std::string m_outputPath;

		int m_colorFormatIndex = 1;
		std::array<ColorFormat, 6> m_colorFormats = { ColorFormat::None, ColorFormat::RGBA8, ColorFormat::RED_INTEGER, ColorFormat::RGBA16F, ColorFormat::BGRA8, ColorFormat::RGB10A2 };
		const std::vector<std::string> m_colorFormatsNames = { "None", "RGBA8", "RED_INTEGER", "RGBA16F", "BGRA8", "RGB10A2" };

		int m_depthFormatIndex = 2;
		std::array<DepthStencilFormat, 5> m_depthFormats = { DepthStencilFormat::None, DepthStencilFormat::DEPTH24_STENCIL8, DepthStencilFormat::DEPTH32F, DepthStencilFormat::DEPTH32F_STENCIL8, DepthStencilFormat::DEPTH16 };
		const std::vector<std::string> m_depthFormatsNames = { "None", "DEPTH24_STENCIL8", "DEPTH32F", "DEPTH32F_STENCIL8", "DEPTH16" };

		bool m_depthTest = true;
		bool m_depthWrite = true;

		int m_depthCompareIndex = 1;
		std::array<DepthCompare, 8> m_depthCompares = { DepthCompare::Never, DepthCompare::Less, DepthCompare::Equal, DepthCompare::LessEqual, DepthCompare::Greater, DepthCompare::NotEqual, DepthCompare::GreaterEqual, DepthCompare::Always };
		const std::vector<std::string> m_depthCompareNames = { "Never", "Less", "Equal", "Less Equal", "Greater", "Not Equal", "Greater Equal", "Always" };

		bool m_stencilEnabled = false;
		int m_sampleCount = 1;

		int m_cullModeIndex = 2;
		std::array<CullMode, 3> m_cullModes = { CullMode::None, CullMode::Front, CullMode::Back };
		const std::vector<std::string> m_cullModesNames = { "None", "Front", "Back" };

		int m_topologyIndex = 3;
		std::array<PrimitiveTopology, 5> m_topologies = { PrimitiveTopology::PointList, PrimitiveTopology::LineList, PrimitiveTopology::LineStrip, PrimitiveTopology::TriangleList, PrimitiveTopology::TriangleStrip };
		const std::vector<std::string> m_topologiesNames = { "Point List", "Line List", "Line Strip", "Triangle List", "Triangle Strip" };

		int m_renderTargetsCount = 1;

		std::vector<BufferElement> m_bufferElements;
		const std::vector<std::string> m_shaderDataTypeNames = { "None", "Float", "Float2", "Float3", "Float4", "Int", "Int2", "Int3", "Int4", "Bool" };

		void resetInputs();

	};

}
