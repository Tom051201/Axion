#include "AxAnimationClip.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

#include "AxionEngine/Source/scene/Animation.h"

#include "AxionAssetPipeline/Source/importer/GLTFImporter.h"

namespace Axion::AAP {

	void AnimationClipParser::createTextFile(const AnimationClipAssetData& data, const std::filesystem::path& outputPath) {

		Ref<AnimationClip> clip = GLTFImporter::extractAnimation(AssetManager::getAbsolute(data.filePath));

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Version" << YAML::Value << ASSET_VERSION_ANIMATION_CLIP;
		out << YAML::Key << "Name" << YAML::Value << data.name;
		out << YAML::Key << "UUID" << YAML::Value << data.uuid;
		out << YAML::Key << "Type" << YAML::Value << "Animation";
		out << YAML::Key << "Source" << YAML::Value << data.filePath.generic_string();

		out << YAML::EndMap;

		std::ofstream fout(outputPath);
		fout << out.c_str();
		AX_CORE_LOG_TRACE("Created .axanim file ({})", outputPath.string());
	}

	void AnimationClipParser::createBinaryFile(const AnimationClipAssetData& data, const std::filesystem::path& outputPath) {

		Ref<AnimationClip> clip = GLTFImporter::extractAnimation(AssetManager::getAbsolute(data.filePath));

		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out) {
			AX_CORE_LOG_ERROR("Failed to create binary file: {}", outputPath.string());
			return;
		}

		// -- Write Header --
		BinaryAssetHeader header;
		header.type = AssetType::AnimationClip;
		header.uuid = data.uuid;
		header.version = ASSET_VERSION_ANIMATION_CLIP;
		out.write(reinterpret_cast<const char*>(&header), sizeof(BinaryAssetHeader));

		// -- Write Base Animation Data --
		uint32_t boneAnimationCount = static_cast<uint32_t>(clip->boneAnimations.size());
		out.write(reinterpret_cast<const char*>(&clip->duration), sizeof(float));
		out.write(reinterpret_cast<const char*>(&clip->ticksPerSecond), sizeof(float));
		out.write(reinterpret_cast<const char*>(&boneAnimationCount), sizeof(uint32_t));

		// -- Write Keyframes --
		for (const auto& boneAnimation : clip->boneAnimations) {
			// -- Write Name --
			uint32_t nameLength = static_cast<uint32_t>(boneAnimation.boneName.size());
			out.write(reinterpret_cast<const char*>(&nameLength), sizeof(uint32_t));
			out.write(boneAnimation.boneName.data(), nameLength);

			// -- Write Positions --
			uint32_t posCount = static_cast<uint32_t>(boneAnimation.positions.size());
			out.write(reinterpret_cast<const char*>(&posCount), sizeof(uint32_t));
			for (uint32_t k = 0; k < posCount; ++k) {
				out.write(reinterpret_cast<const char*>(&boneAnimation.positions[k].time), sizeof(float));
				out.write(reinterpret_cast<const char*>(&boneAnimation.positions[k].value), sizeof(DirectX::XMFLOAT3));
			}

			// -- Write Rotations --
			uint32_t rotCount = static_cast<uint32_t>(boneAnimation.rotations.size());
			out.write(reinterpret_cast<const char*>(&rotCount), sizeof(uint32_t));
			for (uint32_t k = 0; k < rotCount; ++k) {
				out.write(reinterpret_cast<const char*>(&boneAnimation.rotations[k].time), sizeof(float));
				out.write(reinterpret_cast<const char*>(&boneAnimation.rotations[k].value), sizeof(DirectX::XMFLOAT4));
			}

			// -- Write Scales --
			uint32_t scaCount = static_cast<uint32_t>(boneAnimation.scales.size());
			out.write(reinterpret_cast<const char*>(&scaCount), sizeof(uint32_t));
			for (uint32_t k = 0; k < scaCount; ++k) {
				out.write(reinterpret_cast<const char*>(&boneAnimation.scales[k].time), sizeof(float));
				out.write(reinterpret_cast<const char*>(&boneAnimation.scales[k].value), sizeof(DirectX::XMFLOAT3));
			}
		}

		out.close();
		AX_CORE_LOG_TRACE("Baked binary animation clip to {}", outputPath.string());
	}

}
