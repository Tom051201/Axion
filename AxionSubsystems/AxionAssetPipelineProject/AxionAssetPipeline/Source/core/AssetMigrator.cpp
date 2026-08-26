#include "AssetMigrator.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/PathResolver.h"
#include "AxionEngine/Source/core/AssetVersions.h"

#include "AxionAssetPipeline/Source/core/BaseIncludes.h"

namespace Axion::AAP {

	void AssetMigrator::upgradeLegacyAssets(const Shared<Project>& project) {
		if (!project) return;

		auto registry = project->getAssetRegistry()->getMap();
		int upgradedCount = 0;

		for (const auto& [uuid, metadata] : registry) {
			std::string ext = metadata.filePath.extension().string();
			if (ext == ".axmesh" || ext == ".axtex" || ext == ".axaudio" ||
				ext == ".axanim" || ext == ".axskelmesh" || ext == ".axtcube" || ext == ".axshader") {
				std::filesystem::path absPath = PathResolver::resolve(metadata.filePath.generic_string());

				std::ifstream stream(absPath);
				if (!stream.is_open()) continue;

				YAML::Node data;
				try { data = YAML::Load(stream); }
				catch (...) { continue; }
				stream.close();

				uint32_t version = data["Version"] ? data["Version"].as<uint32_t>() : 1;

				if (version == 1 && data["Source"]) {
					std::string rawSource = data["Source"].as<std::string>();
					if (!rawSource.empty() && rawSource[0] != '{' && !std::filesystem::path(rawSource).is_absolute()) {

						// -- Update The Path --
						data["Source"] = "{assetsdir}/" + rawSource;

						// -- Bump The Version --
						if (ext == ".axmesh") data["Version"] = ASSET_VERSION_MESH;
						else if (ext == ".axtex") data["Version"] = ASSET_VERSION_TEXTURE2D;
						else if (ext == ".axaudio") data["Version"] = ASSET_VERSION_AUDIO;
						else if (ext == ".axanim") data["Version"] = ASSET_VERSION_ANIMATION_CLIP;
						else if (ext == ".axskelmesh") data["Version"] = ASSET_VERSION_SKELETAL_MESH;
						else if (ext == ".axtcube") data["Version"] = ASSET_VERSION_TEXTURE_CUBE;
						else if (ext == ".axshader") data["Version"] = ASSET_VERSION_SHADER;

						// -- Save --
						YAML::Emitter out;
						out << data;
						std::ofstream fout(absPath);
						fout << out.c_str();

						upgradedCount++;
					}
				}
			}
		}

		if (upgradedCount > 0) {
			AX_CORE_LOG_INFO("Asset Migrator: Upgraded {} legacy assets to VFS format on disk!", upgradedCount);
		}
	}

}
