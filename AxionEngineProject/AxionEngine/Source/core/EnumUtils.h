#pragma once

#include "AxionEngine/Source/render/Formats.h"
#include "AxionEngine/Source/render/Pipeline.h"
#include "AxionEngine/Source/render/Buffers.h"
#include "AxionEngine/Source/audio/AudioClip.h"

namespace Axion {

	class EnumUtils {
	public:

		// --- CullMode ---
		inline static const char* toString(Axion::CullMode mode) {
			switch (mode) {
			case Axion::CullMode::None:		return "None";
			case Axion::CullMode::Front:	return "Front";
			case Axion::CullMode::Back:		return "Back";
			}
			return "Unknown";
		}

		inline static Axion::CullMode cullModeFromString(const std::string& str) {
			if (str == "None")	return Axion::CullMode::None;
			if (str == "Front")	return Axion::CullMode::Front;
			if (str == "Back")	return Axion::CullMode::Back;
			throw std::invalid_argument("Invalid CullMode string: " + str);
		}



		// --- DepthCompare ---
		inline static const char* toString(Axion::DepthCompare mode) {
			switch (mode) {
				case Axion::DepthCompare::Never:		return "Never";
				case Axion::DepthCompare::Less:			return "Less";
				case Axion::DepthCompare::Equal:		return "Equal";
				case Axion::DepthCompare::LessEqual:	return "LessEqual";
				case Axion::DepthCompare::Greater:		return "Greater";
				case Axion::DepthCompare::NotEqual:		return "NotEqual";
				case Axion::DepthCompare::GreaterEqual:	return "GreaterEqual";
				case Axion::DepthCompare::Always:		return "Always";
			}
			return "Unknown";
		}

		inline static Axion::DepthCompare depthCompareFromString(const std::string& str) {
			if (str == "Never")			return Axion::DepthCompare::Never;
			if (str == "Less")			return Axion::DepthCompare::Less;
			if (str == "Equal")			return Axion::DepthCompare::Equal;
			if (str == "LessEqual")		return Axion::DepthCompare::LessEqual;
			if (str == "Greater")		return Axion::DepthCompare::Greater;
			if (str == "NotEqual")		return Axion::DepthCompare::NotEqual;
			if (str == "GreaterEqual")	return Axion::DepthCompare::GreaterEqual;
			if (str == "Always")		return Axion::DepthCompare::Always;
			throw std::invalid_argument("Invalid DepthCompare string: " + str);
		}



		// --- PrimitiveTopology ---
		inline static const char* toString(Axion::PrimitiveTopology topo) {
			switch (topo) {
			case Axion::PrimitiveTopology::PointList:		return "PointList";
			case Axion::PrimitiveTopology::LineList:		return "LineList";
			case Axion::PrimitiveTopology::LineStrip:		return "LineStrip";
			case Axion::PrimitiveTopology::TriangleList:	return "TriangleList";
			case Axion::PrimitiveTopology::TriangleStrip:	return "TriangleStrip";
			}
			return "Unknown";
		}

		inline static Axion::PrimitiveTopology primitiveTopologyFromString(const std::string& str) {
			if (str == "PointList")			return Axion::PrimitiveTopology::PointList;
			if (str == "LineList")			return Axion::PrimitiveTopology::LineList;
			if (str == "LineStrip")			return Axion::PrimitiveTopology::LineStrip;
			if (str == "TriangleList")		return Axion::PrimitiveTopology::TriangleList;
			if (str == "TriangleStrip")		return Axion::PrimitiveTopology::TriangleStrip;
			throw std::invalid_argument("Invalid PrimitiveTopology string: " + str);
		}



		// --- ColorFormat ---
		inline static const char* toString(Axion::ColorFormat fmt) {
			switch (fmt) {
			case Axion::ColorFormat::None:			return "None";
			case Axion::ColorFormat::RGBA8:			return "RGBA8";
			case Axion::ColorFormat::RED_INTEGER:	return "RED_INTEGER";
			case Axion::ColorFormat::RGBA16F:		return "RGBA16F";
			case Axion::ColorFormat::BGRA8:			return "BGRA8";
			case Axion::ColorFormat::RGB10A2:		return "RGB10A2";
			}
			return "Unknown";
		}

		inline static Axion::ColorFormat colorFormatFromString(const std::string& str) {
			if (str == "None")			return Axion::ColorFormat::None;
			if (str == "RGBA8")			return Axion::ColorFormat::RGBA8;
			if (str == "RED_INTEGER")	return Axion::ColorFormat::RED_INTEGER;
			if (str == "RGBA16F")		return Axion::ColorFormat::RGBA16F;
			if (str == "BGRA8")			return Axion::ColorFormat::BGRA8;
			if (str == "RGB10A2")		return Axion::ColorFormat::RGB10A2;
			throw std::invalid_argument("Invalid ColorFormat string: " + str);
		}



		// --- DepthStencilFormat ---
		inline static const char* toString(Axion::DepthStencilFormat fmt) {
			switch (fmt) {
			case Axion::DepthStencilFormat::None:				return "None";
			case Axion::DepthStencilFormat::DEPTH24_STENCIL8:	return "DEPTH24_STENCIL8";
			case Axion::DepthStencilFormat::DEPTH32F:			return "DEPTH32F";
			case Axion::DepthStencilFormat::DEPTH32F_STENCIL8:	return "DEPTH32F_STENCIL8";
			case Axion::DepthStencilFormat::DEPTH16:			return "DEPTH16";
			}
			return "Unknown";
		}

		inline static Axion::DepthStencilFormat depthStencilFormatFromString(const std::string& str) {
			if (str == "None")				return Axion::DepthStencilFormat::None;
			if (str == "DEPTH24_STENCIL8")	return Axion::DepthStencilFormat::DEPTH24_STENCIL8;
			if (str == "DEPTH32F")			return Axion::DepthStencilFormat::DEPTH32F;
			if (str == "DEPTH32F_STENCIL8")	return Axion::DepthStencilFormat::DEPTH32F_STENCIL8;
			if (str == "DEPTH16")			return Axion::DepthStencilFormat::DEPTH16;
			throw std::invalid_argument("Invalid DepthStencilFormat string: " + str);
		}



		// --- ShaderDataType ---
		inline static const char* toString(Axion::ShaderDataType type) {
			switch (type) {
			case Axion::ShaderDataType::None:   return "None";
			case Axion::ShaderDataType::Float:  return "Float";
			case Axion::ShaderDataType::Float2: return "Float2";
			case Axion::ShaderDataType::Float3: return "Float3";
			case Axion::ShaderDataType::Float4: return "Float4";
			case Axion::ShaderDataType::Int:    return "Int";
			case Axion::ShaderDataType::Int2:   return "Int2";
			case Axion::ShaderDataType::Int3:   return "Int3";
			case Axion::ShaderDataType::Int4:   return "Int4";
			case Axion::ShaderDataType::Bool:   return "Bool";
			}
			return "Unknown";
		}

		inline static Axion::ShaderDataType shaderDataTypeFromString(const std::string& str) {
			if (str == "None")   return Axion::ShaderDataType::None;
			if (str == "Float")  return Axion::ShaderDataType::Float;
			if (str == "Float2") return Axion::ShaderDataType::Float2;
			if (str == "Float3") return Axion::ShaderDataType::Float3;
			if (str == "Float4") return Axion::ShaderDataType::Float4;
			if (str == "Int")    return Axion::ShaderDataType::Int;
			if (str == "Int2")   return Axion::ShaderDataType::Int2;
			if (str == "Int3")   return Axion::ShaderDataType::Int3;
			if (str == "Int4")   return Axion::ShaderDataType::Int4;
			if (str == "Bool")   return Axion::ShaderDataType::Bool;
			throw std::invalid_argument("Invalid ShaderDataType string: " + str);
		}



		// --- Audio Mode ---
		inline static const char* toString(AudioClip::Mode mode) {
			switch (mode) {
			case AudioClip::Mode::Memory: return "Memory";
			case AudioClip::Mode::Stream: return "Stream";
			}
			return "Unknown";
		}

		inline static AudioClip::Mode AudioClipModeFromString(const std::string& str) {
			if (str == "Memory") return AudioClip::Mode::Memory;
			if (str == "Stream") return AudioClip::Mode::Stream;
			throw std::invalid_argument("Invalid AudioClip::Mode string: " + str);
		}

	};

}
