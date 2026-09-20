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

			// -- Load Editor Settings --
			if (auto editor = settings["Editor"]) {
				if (editor["StartupProject"]) {
					EditorSettings::startupProjectPath = editor["StartupProject"].as<std::string>();
				}

				if (editor["MaxAssetsPerFrame"]) {
					AssetManager::setMaxAssetsPerFrame(editor["MaxAssetsPerFrame"].as<uint32_t>());
				}

				if (editor["EnableDiscordRPC"]) {
					EditorSettings::enableDiscordRPC = editor["EnableDiscordRPC"].as<bool>();
				}

				if (editor["OpenTextEditors"]) {
					for (auto pathNode : editor["OpenTextEditors"]) {
						std::string pathStr = pathNode.as<std::string>();
						if (std::filesystem::exists(pathStr)) {
							EditorSettings::openTextEditors.push_back(pathStr);
						}
					}
				}
			}

			// -- Load Panel Settings --
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

		// -- Editor Settings --
		out << YAML::Key << "Editor" << YAML::BeginMap;

		if (!EditorSettings::startupProjectPath.empty() && EditorSettings::startupProjectPath != "None") {
			out << YAML::Key << "StartupProject" << YAML::Value << EditorSettings::startupProjectPath.generic_string();
		}
		out << YAML::Key << "EnableDiscordRPC" << YAML::Value << EditorSettings::enableDiscordRPC;
		out << YAML::Key << "MaxAssetsPerFrame" << YAML::Value << AssetManager::getMaxAssetsPerFrame();
		// -- Open Text Editors --
		out << YAML::Key << "OpenTextEditors" << YAML::Value << YAML::BeginSeq;
		for (const auto& pathStr : EditorSettings::openTextEditors) {
			std::filesystem::path path = pathStr;
			out << path.generic_string();
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		// -- Panel Settings --
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

	// -- Viewport Panel --
	bool EditorSettings::viewportPanelShowRendererStats = true;
	bool EditorSettings::viewportPanelInvertCameraY = false;
	bool EditorSettings::viewportPanelInvertCameraX = false;
	float EditorSettings::viewportPanelGizmoScale = 1.0f;

}
