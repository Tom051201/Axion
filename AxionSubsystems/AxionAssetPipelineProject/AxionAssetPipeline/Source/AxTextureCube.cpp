#include "AxTextureCube.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void TextureCubeParser::createTextFile(const TextureCubeAssetData& data, const std::filesystem::path& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_TEXTURE_CUBE;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();
		out << YAML::Key << "Type" << YAML::Value << "TextureCube";
		out << YAML::Key << "Format" << YAML::Value << FormatUtils::textureFormatToString(data.fileFormat);
		out << YAML::Key << "Source" << YAML::Value << data.filePath.generic_string();

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axtcube file ({})", outputPath.string());
	}

	void TextureCubeParser::createBinaryFile(const TextureCubeAssetData& data, const std::filesystem::path& outputPath) {
		std::ifstream imageFile(data.filePath, std::ios::in | std::ios::binary);
		if (!imageFile) {
			AX_CORE_LOG_ERROR("Failed to open source image: {}", data.filePath.string());
			return;
		}

		imageFile.seekg(0, std::ios::end);
		size_t fileSize = imageFile.tellg();
		imageFile.seekg(0, std::ios::beg);

		std::vector<uint8_t> fileData(fileSize);
		imageFile.read(reinterpret_cast<char*>(fileData.data()), fileSize);
		imageFile.close();

		std::ofstream out(outputPath, std::ios::out | std::ios::binary);

		// -- Write Header --
		BinaryAssetHeader header = {};
		header.type = AssetType::TextureCube;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_TEXTURE_CUBE;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		// -- Write Data --
		uint64_t dataSize = static_cast<uint64_t>(fileSize);
		out.write(reinterpret_cast<const char*>(&dataSize), sizeof(uint64_t));
		out.write(reinterpret_cast<const char*>(fileData.data()), fileSize);

		out.close();
		AX_CORE_LOG_TRACE("Baked binary TextureCube to {}", outputPath.string());
	}

}
