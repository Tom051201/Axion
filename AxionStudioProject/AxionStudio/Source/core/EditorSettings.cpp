#include "studiopch.h"
#include "EditorSettings.h"

#include <yaml-cpp/yaml.h>

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/AssetManager.h"

namespace Axion {

	// -- Loading and saving --
	bool EditorSettings::load(const std::filesystem::path& path, std::function<void(const YAML::Node&)> deserializeSubPanels) {
		if (!std::filesystem::exists(path)) {
			AX_CORE_LOG_ERROR("Failed to open Editor Settings file! Using defaults!");
			return false;
		}

		try {
			YAML::Node settings = YAML::LoadFile(path.string());

			if (settings["StartupProject"]) {
				EditorSettings::startupProjectPath = settings["StartupProject"].as<std::string>();
			}

			if (settings["MaxAssetsPerFrame"]) {
				AssetManager::setMaxAssetsPerFrame(settings["MaxAssetsPerFrame"].as<uint32_t>());
			}

			if (settings["EnableDiscordRPC"]) {
				EditorSettings::enableDiscordRPC = settings["EnableDiscordRPC"].as<bool>();
			}

			if (settings["MaterialEditorInvertCamera"]) {
				EditorSettings::materialEditorInvertCamera = settings["MaterialEditorInvertCamera"].as<bool>();
			}

			if (settings["OpenTextEditors"]) {
				for (auto pathNode : settings["OpenTextEditors"]) {
					std::string pathStr = pathNode.as<std::string>();
					if (std::filesystem::exists(pathStr)) {
						EditorSettings::openTextEditors.push_back(pathStr);
					}
				}
			}

			if (settings["AssetLibraryPaths"]) {
				std::vector<std::string> savedPaths;
				for (auto pathNode : settings["AssetLibraryPaths"]) {
					savedPaths.push_back(pathNode.as<std::string>());
				}
				EditorSettings::assetLibraryPaths = savedPaths;
			}

			if (deserializeSubPanels) {
				deserializeSubPanels(settings);
			}

		}
		catch (const YAML::Exception& ex) {
			AX_CORE_LOG_ERROR("Failed to parse Editor Settings file: {0}", ex.what());
			return false;
		}

		return true;
	}

	bool EditorSettings::save(const std::filesystem::path& path, std::function<void(YAML::Emitter&)> serializeSubPanels) {
		YAML::Emitter out;
		out << YAML::BeginMap;

		if (!EditorSettings::startupProjectPath.empty() && EditorSettings::startupProjectPath != "None") {
			out << YAML::Key << "StartupProject" << YAML::Value << EditorSettings::startupProjectPath.generic_string();
		}

		out << YAML::Key << "MaxAssetsPerFrame" << YAML::Value << AssetManager::getMaxAssetsPerFrame();
		out << YAML::Key << "EnableDiscordRPC" << YAML::Value << EditorSettings::enableDiscordRPC;
		out << YAML::Key << "MaterialEditorInvertCamera" << YAML::Value << EditorSettings::materialEditorInvertCamera;

		// -- Open Text Editors --
		out << YAML::Key << "OpenTextEditors" << YAML::Value << YAML::BeginSeq;
		for (const auto& pathStr : EditorSettings::openTextEditors) {
			out << pathStr;
		}
		out << YAML::EndSeq;

		// -- Asset Library Paths --
		out << YAML::Key << "AssetLibraryPaths" << YAML::Value << YAML::BeginSeq;
		for (const auto& pathStr : EditorSettings::assetLibraryPaths) {
			out << pathStr;
		}
		out << YAML::EndSeq;

		if (serializeSubPanels) {
			serializeSubPanels(out);
		}

		out << YAML::EndMap;

		std::filesystem::create_directories(path.parent_path());
		std::ofstream fout(path);
		if (fout.is_open()) {
			fout << out.c_str();
			fout.close();
			return true;
		}

		AX_CORE_LOG_ERROR("Failed to write Editor Settings file!");
		return false;
	}




	// -- Editor --
	std::filesystem::path EditorSettings::startupProjectPath = "None";
	std::vector<std::string> EditorSettings::openTextEditors = {};
	bool EditorSettings::enableDiscordRPC = true;

	// -- Content Browser Panel --
	bool EditorSettings::contentBrowserShowContentArea = true;
	bool EditorSettings::contentBrowserShowVFSTree = true;
	bool EditorSettings::contentBrowserShowPhysicalTree = true;

	// -- Material Panel --
	bool EditorSettings::materialEditorInvertCamera = false;

	// -- Asset Library --
	std::vector<std::string> EditorSettings::assetLibraryPaths = {};

}
