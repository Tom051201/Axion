#include "AxTexture2D.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void Texture2DParser::createTextFile(const Texture2DAssetData& data, const std::filesystem::path& outputPath) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_TEXTURE2D;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid.toString();
		out << YAML::Key << "Type" << YAML::Value << "Texture2D";
		out << YAML::Key << "Format" << YAML::Value << data.fileFormat;
		out << YAML::Key << "Source" << YAML::Value << data.filePath.generic_string();

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axtex file ({})", outputPath.string());
	}

	void Texture2DParser::createBinaryFile(const Texture2DAssetData& data, const std::filesystem::path& outputPath) {
		// -- Read Source Image File -- 
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

		// -- Open Output Binary File --
		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
			return;
		}

		// -- Write Header --
		BinaryAssetHeader header = {};
		header.type = AssetType::Texture2D;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_TEXTURE2D;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		// -- Write Data Size and Raw File Bytes --
		uint64_t dataSize = static_cast<uint64_t>(fileSize);
		out.write(reinterpret_cast<const char*>(&dataSize), sizeof(uint64_t));
		out.write(reinterpret_cast<const char*>(fileData.data()), fileSize);

		out.close();
		AX_CORE_LOG_TRACE("Baked binary Texture2D to {}", outputPath.string());
	}

}
