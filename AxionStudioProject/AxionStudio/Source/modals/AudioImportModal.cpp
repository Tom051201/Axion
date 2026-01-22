#include "AudioImportModal.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/audio/AudioManager.h"

#include "AxionAssetPipeline/Source/AxAudio.h"

namespace Axion {

	constexpr float inputFieldWidth = 200.0f;

	AudioImportModal::AudioImportModal(const char* name) : Modal(name) {}

	AudioImportModal::~AudioImportModal() {}

	void AudioImportModal::close() {
		Modal::close();
		clearBuffers();
	}

	void AudioImportModal::presetFromFile(const std::filesystem::path& sourceFile) {
		clearBuffers();

		// -- Source Path --
		std::string abs = sourceFile.string();
		strcpy_s(m_sourcePathBuffer, IM_ARRAYSIZE(m_sourcePathBuffer), abs.c_str());

		// -- Default output folder --
		auto audioDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "audio";
		strcpy_s(m_outputPathBuffer, IM_ARRAYSIZE(m_outputPathBuffer), audioDir.string().c_str());

		// -- Default name --
		std::string name = sourceFile.stem().string();
		strcpy_s(m_nameBuffer, IM_ARRAYSIZE(m_nameBuffer), name.c_str());

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
		if (formatStr == ".mp3") {
			m_importFormat = 0;
		}
		else if (formatStr == ".wav") {
			m_importFormat = 1;
		}
		else {
			AX_CORE_LOG_WARN("Unable to identify automatically format of audio");
		}
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
			ImGui::InputText("##AudioName_input", m_nameBuffer, sizeof(m_nameBuffer));


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
			ImGui::InputText("##AudioSourcePath_input", m_sourcePathBuffer, sizeof(m_sourcePathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##AudioSourceFile_button")) {
				std::filesystem::path audioDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "audio";
				std::string absPath = FileDialogs::openFile({ {"Audio Files", "*.mp3;*.wav;*.ogg"} }, audioDir.string());
				if (!absPath.empty()) {
					strcpy_s(m_sourcePathBuffer, IM_ARRAYSIZE(m_sourcePathBuffer), absPath.c_str());
					m_sourcePathBuffer[IM_ARRAYSIZE(m_sourcePathBuffer) - 1] = '\0';
				}
			}


			// -- Output path --
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Output Location");
			ImGui::TableSetColumnIndex(1);
			ImGui::SetNextItemWidth(inputFieldWidth);
			ImGui::InputText("##AudioOutputPath_input", m_outputPathBuffer, sizeof(m_outputPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse##AudioOutputDir_button")) {
				std::filesystem::path audioDir = std::filesystem::path(ProjectManager::getProject()->getAssetsPath()) / "audio";
				std::string absPath = FileDialogs::openFolder(audioDir.string());
				if (!absPath.empty()) {
					strcpy_s(m_outputPathBuffer, IM_ARRAYSIZE(m_outputPathBuffer), absPath.c_str());
					m_outputPathBuffer[IM_ARRAYSIZE(m_outputPathBuffer) - 1] = '\0';
				}
			}

			ImGui::EndTable();

			// -- Validate input --
			std::filesystem::path sourceFilePath = std::string(m_sourcePathBuffer);
			bool validSource = std::filesystem::exists(sourceFilePath);

			std::filesystem::path outputDirPath = std::string(m_outputPathBuffer);
			bool validOutputPath = std::filesystem::exists(outputDirPath);
			bool validOutputFile = !std::filesystem::exists(outputDirPath / (std::string(m_nameBuffer) + ".axaudio"));

			bool disabled = (
				strlen(m_nameBuffer) == 0 ||
				strlen(m_sourcePathBuffer) == 0 ||
				strlen(m_outputPathBuffer) == 0 ||
				!validSource ||
				!validOutputPath ||
				!validOutputFile
			);

			ImGui::Separator();
			ImGui::BeginDisabled(disabled);
			if (ImGui::Button("Create")) {
				std::filesystem::path outDir = std::string(m_outputPathBuffer);
				std::filesystem::path outFile = outDir / (std::string(m_nameBuffer) + ".axaudio");

				AAP::AudioAssetData data;
				data.name = m_nameBuffer;
				data.fileFormat = m_formatNames[m_importFormat];
				data.audioFilePath = AssetManager::getRelativeToAssets(std::string(m_sourcePathBuffer));
				data.mode = m_types[m_loadType];

				AAP::AudioParser::createAxAudioFile(data, outFile.string());
				close();
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				close();
			}
		}
	}

	void AudioImportModal::clearBuffers() {
		m_nameBuffer[0] = '\0';
		m_sourcePathBuffer[0] = '\0';
		m_outputPathBuffer[0] = '\0';
		m_loadType = 0;
		m_importFormat = 0;
	}

}
