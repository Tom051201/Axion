#pragma once

#include <string>

namespace Axion::AAP {

	enum class MeshFormat {
		None = 0,
		OBJ,
		GLTF,
		GLB
	};

	enum class TextureFormat {
		None = 0,
		PNG,
		JPG,
		HDR
	};

	enum class AudioFormat {
		None = 0,
		WAV,
		MP3,
		OGG
	};

	enum class ShaderFormat {
		None = 0,
		HLSL,
		GLSL
	};

	class FormatUtils {
	public:

		// ----- Meshes -----
		static MeshFormat meshFormatFromString(const std::string& formatString) {
			std::string upper = formatString;
			for (auto& c : upper) c = toupper(c);

			if (upper == "OBJ") return MeshFormat::OBJ;
			if (upper == "GLTF") return MeshFormat::GLTF;
			if (upper == "GLB") return MeshFormat::GLB;
			return MeshFormat::None;
		}

		static std::string meshFormatToString(MeshFormat format) {
			switch (format) {
				case MeshFormat::OBJ: return "OBJ";
				case MeshFormat::GLTF: return "GLTF";
				case MeshFormat::GLB: return "GLB";
				default: return "Unknown";
			}
		}

		// ----- Texture Formats -----
		static TextureFormat textureFormatFromString(const std::string& formatString) {
			std::string upper = formatString;
			for (auto& c : upper) c = toupper(c);

			if (upper == "PNG") return TextureFormat::PNG;
			if (upper == "JPG" || upper == "JPEG") return TextureFormat::JPG;
			if (upper == "HDR") return TextureFormat::HDR;
			return TextureFormat::None;
		}

		static std::string textureFormatToString(TextureFormat format) {
			switch (format) {
				case TextureFormat::PNG: return "PNG";
				case TextureFormat::JPG: return "JPG";
				case TextureFormat::HDR: return "HDR";
				default: return "Unknown";
			}
		}

		// ----- Audio Formats -----
		static AudioFormat audioFormatFromString(const std::string& formatString) {
			std::string upper = formatString;
			for (auto& c : upper) c = toupper(c);

			if (upper == "WAV") return AudioFormat::WAV;
			if (upper == "MP3") return AudioFormat::MP3;
			if (upper == "OGG") return AudioFormat::OGG;
			return AudioFormat::None;
		}

		static std::string audioFormatToString(AudioFormat format) {
			switch (format) {
				case AudioFormat::WAV: return "WAV";
				case AudioFormat::MP3: return "MP3";
				case AudioFormat::OGG: return "OGG";
				default: return "Unknown";
			}
		}

		// ----- Shader Formats -----
		static ShaderFormat shaderFormatFromString(const std::string& formatString) {
			std::string upper = formatString;
			for (auto& c : upper) c = toupper(c);

			if (upper == "HLSL") return ShaderFormat::HLSL;
			if (upper == "GLSL") return ShaderFormat::GLSL;
			return ShaderFormat::None;
		}

		static std::string shaderFormatToString(ShaderFormat format) {
			switch (format) {
				case ShaderFormat::HLSL: return "HLSL";
				case ShaderFormat::GLSL: return "GLSL";
				default: return "Unknown";
			}
		}

	};

}
