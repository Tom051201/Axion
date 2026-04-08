#include "AudioImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/AssetVersions.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/audio/AudioManager.h"

#include "AxionAssetPipeline/Source/AxAudio.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	void AudioImportModal::presetFromFile(const std::filesystem::path& sourceFile) {
		resetInputs();

		// -- Source Path --
		m_sourcePath = sourceFile.string();

		// -- Default output folder --
		std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "audio";
		m_outputPath = audioDir.string();

		// -- Default name --
		m_name = sourceFile.stem().string();

		// -- Load type --
		AudioFileInfo fileInfo;
		bool success = AudioManager::readAudioFileMetadata(sourceFile, fileInfo);
		AudioClip::Mode mode = AudioClip::Mode::Stream;
		if (success) {
			mode = AudioManager::decideMode(fileInfo);
			if (mode == AudioClip::Mode::Memory) {
				m_loadType = 1;
			}
		}

		// -- Format --
		std::string formatStr = sourceFile.extension().string();
		std::transform(formatStr.begin(), formatStr.end(), formatStr.begin(), [](unsigned char c) { return std::tolower(c); });
		if (formatStr == ".mp3") m_importFormat = 0;
		else if (formatStr == ".wav") m_importFormat = 1;
		else if (formatStr == ".ogg") m_importFormat = 2;
		else AX_CORE_LOG_WARN("Unable to identify automatically format of audio");
	}

	void AudioImportModal::renderContent() {

		ImGui::SeparatorText("Import Audio Asset");
		ImGui::Spacing();

		if (ImGui::BeginTable("##ImportAudioTable", 2, ImGuiTableFlags_BordersInnerV)) {
			ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

			// -- Name --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Name");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##AudioName_input", &m_name);


			// -- Format --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Format");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##AudioFormat_combo", &m_importFormat, m_formatNames, IM_ARRAYSIZE(m_formatNames));


			// -- Type --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Type");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::Combo("##AudioLoad_combo", &m_loadType, m_typesNames, IM_ARRAYSIZE(m_typesNames));


			// -- Source path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Source File");
			ImGui::Separator();
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##AudioSourcePath_input", &m_sourcePath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##AudioSourceFile_button")) {
				std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "audio";
				std::filesystem::path absPath;
				if (std::filesystem::exists(audioDir)) {
					absPath = FileDialogs::openFile({ {"Audio Files", "*.mp3;*.wav;*.ogg"} }, audioDir);
				}
				else {
					absPath = FileDialogs::openFile({ {"Audio Files", "*.mp3;*.wav;*.ogg"} }, ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_sourcePath = absPath.string();
			}


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##AudioOutputPath_input", &m_outputPath);
			ImGui::SameLine();
			if (ImGui::Button("Browse...##AudioOutputDir_button")) {
				std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "audio";
				std::filesystem::path absPath;
				if (std::filesystem::exists(audioDir)) {
					absPath = FileDialogs::openFolder(audioDir);
				}
				else {
					absPath = FileDialogs::openFolder(ProjectManager::getProject()->getAssetsPath());
				}
				if (!absPath.empty()) m_outputPath = absPath.string();
			}

			ImGui::EndTable();

			// -- Validate input --
			std::string finalName = m_name + ".axaudio";
			std::filesystem::path finalPath = std::filesystem::path(m_outputPath) / finalName;

			bool sourceExists = std::filesystem::exists(m_sourcePath);
			bool sourceIsFile = std::filesystem::is_regular_file(m_sourcePath);
			bool outputExists = std::filesystem::exists(m_outputPath);
			bool outputIsDirectory = std::filesystem::is_directory(m_outputPath);
			bool invalidOutFileName = std::filesystem::exists(finalPath);

			bool disabled = (
				m_name.empty() ||
				m_sourcePath.empty() ||
				m_outputPath.empty() ||
				!sourceExists ||
				!sourceIsFile ||
				!outputExists ||
				!outputIsDirectory ||
				invalidOutFileName
			);

			if (disabled) {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
				if (m_name.empty()) ImGui::Text("No Name is set.");
				else if (m_sourcePath.empty()) ImGui::Text("No source file is set.");
				else if (m_outputPath.empty()) ImGui::Text("No output directory is set.");
				else if (!sourceExists) ImGui::Text("Source file does not exist.");
				else if (!sourceIsFile) ImGui::Text("Source is not a file.");
				else if (!outputExists) ImGui::Text("Output directory does not exist.");
				else if (!outputIsDirectory) ImGui::Text("Output is not a directory.");
				else if (invalidOutFileName) ImGui::Text("Asset with this name already exists.");
				ImGui::PopStyleColor();
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(50, 255, 50, 255));
				ImGui::Text("Ready to create asset.");
				ImGui::PopStyleColor();
			}

			ImGui::Separator();
			ImGui::BeginDisabled(disabled);
			if (ImGui::Button("Create")) {
				UUID newAssetUUID = UUID::generate();

				AAP::AudioAssetData data;
				data.uuid = newAssetUUID;
				data.name = m_name;
				data.fileFormat = m_formatNames[m_importFormat];
				data.audioFilePath = AssetManager::getRelativeToAssets(m_sourcePath);
				data.mode = m_types[m_loadType];

				AAP::AudioParser::createTextFile(data, finalPath);

				AssetMetadata metadata;
				metadata.handle = newAssetUUID;
				metadata.type = AssetType::AudioClip;
				metadata.filePath = AssetManager::getRelativeToAssets(finalPath);

				auto registry = ProjectManager::getProject()->getAssetRegistry();
				registry->add(metadata);
				registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");

				close();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				close();
			}

			// -- Version --
			std::string versionText = "v" + std::to_string(ASSET_VERSION_AUDIO);
			float textWidth = ImGui::CalcTextSize(versionText.c_str()).x;
			float windowWidth = ImGui::GetWindowWidth();
			ImGui::SameLine(windowWidth - textWidth - ImGui::GetStyle().WindowPadding.x);
			ImGui::TextDisabled("%s", versionText.c_str());
		}
	}

	void AudioImportModal::resetInputs() {
		m_name.clear();
		m_sourcePath.clear();
		m_outputPath.clear();
		m_loadType = 0;
		m_importFormat = 0;
	}

}
