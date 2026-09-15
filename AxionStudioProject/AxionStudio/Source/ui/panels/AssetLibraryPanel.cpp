#include "studiopch.h"
#include "AssetLibraryPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SImage.h>
#include <Silica/include/SWrapBox.h>
#include <Silica/include/SWrappedTextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SMenuAnchor.h>

#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/YAMLHelper.h"
#include "AxionEngine/Source/core/PathResolver.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/core/EditorSettings.h"
#include "AxionStudio/Source/ui/EditorTheme.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"

namespace {
	constexpr float SEARCH_BAR_WIDTH = 250.0f;
	constexpr float CARD_WIDTH = 220.0f;
	constexpr float CARD_HEIGHT = 300.0f;

	constexpr float THUMBNAIL_HEIGHT = 140.0f;
	constexpr float TITLE_HEIGHT = 20.0f;
	constexpr float DESC_HEIGHT = 60.0f;
	constexpr uint32_t DESC_MAX_LINES = 3;
}

namespace Axion {

	AssetLibraryPanel::AssetLibraryPanel() {
		// -- Sync with global config --
		for (const std::string& pathStr : EditorSettings::assetLibraryPaths) {
			m_libraryPaths.push_back(std::filesystem::path(pathStr));
		}

		if (m_libraryPaths.empty()) {
			std::filesystem::path defaultLib = std::filesystem::current_path() / "AxionStudio" / "Resources" / "DefaultAssets";
			if (!std::filesystem::exists(defaultLib)) std::filesystem::create_directories(defaultLib);

			m_libraryPaths.push_back(defaultLib);
			EditorSettings::assetLibraryPaths.push_back(defaultLib.string());
		}

		scanLibraries();
	}

	void AssetLibraryPanel::addLibraryDirectory(const std::filesystem::path& path) {
		m_libraryPaths.push_back(path);
		scanLibraries();
	}

	void AssetLibraryPanel::scanLibraries() {
		m_availablePacks.clear();

		for (const auto& libPath : m_libraryPaths) {
			if (!std::filesystem::exists(libPath)) continue;

			for (const auto& entry : std::filesystem::directory_iterator(libPath)) {
				if (!entry.is_directory()) continue;

				AssetPack pack;
				pack.name = entry.path().filename().string();
				pack.sourcePath = entry.path();
				pack.description = "A collection of assets ready to be imported into your project.";
				pack.thumbnailID = 0;

				// -- Load Thumbnail --
				std::filesystem::path thumbnailPath = entry.path() / "thumbnail.png";
				if (std::filesystem::exists(thumbnailPath)) {
					Ref<Texture2D> thumbnailTexture = Texture2D::create(thumbnailPath);
					Silica::TextureID thumbnailID = SilicaContext::getTextureID(thumbnailTexture);
					pack.thumbnailTexture = thumbnailTexture;
					pack.thumbnailID = thumbnailID;
				}

				// -- Load Info --
				std::filesystem::path infoPath = entry.path() / "info.yaml";
				if (std::filesystem::exists(infoPath)) {
					try {
						YAML::Node info = YAML::LoadFile(infoPath.string());
						if (info["Name"]) pack.name = info["Name"].as<std::string>();
						if (info["Description"]) pack.description = info["Description"].as<std::string>();
					}
					catch (const YAML::Exception& e) {
						AX_CORE_LOG_WARN("Failed to parse info.yaml in pack {0}: {1}", pack.sourcePath.string(), e.what());
					}
				}

				m_availablePacks.push_back(pack);
			}
		}
	}

	Silica::WidgetPtr AssetLibraryPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({.borderThickness = Silica::GetTheme().Border_Thickness });

			// -- Pack Count --
			m_packCountText = Silica::MakeWidget<Silica::STextBlock>({
				.text = "0 Packs Found",
				.color = Silica::GetTheme().Text_Dim
			});

			// -- Search Bar --
			auto searchBar = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{ SEARCH_BAR_WIDTH, 0.0f },
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = m_searchQuery,
					.hintText = "Search Packs...",
					.onTextChanged = [this](const std::string& val) {
						m_searchQuery = val;
						rebuildUI();
					}
				})
			});

			// -- Options Menu --
			auto optionsMenu = Silica::MakeWidget<Silica::SAlign>({
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::SMenuAnchor>({
					.openOnHover = false,
					.openToRight = true,
					.anchorContent = Silica::MakeWidget<Silica::SButton>({
						.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
						.color = Silica::Color::transparent(),
						.hoverColor = Silica::Color(255, 255, 255, 20),
						.onClick = []() { return Silica::EventReply::unhandled(); },
						.child = Silica::MakeWidget<Silica::SImage>({
							.textureID = SilicaContext::getIcon("GearIcon"),
							.tint = Silica::GetTheme().Text_Main,
							.desiredSize = { EditorTheme::ICON_SIZE_SMALL, EditorTheme::ICON_SIZE_SMALL }
						})
					}),
					.menuContent = Silica::MakeWidget<Silica::SBox>({
						.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
						.explicitSize = Silica::Vec2{ EditorTheme::OPTIONS_MENU_WIDTH, 0.0f },
						.borderThickness = Silica::GetTheme().Border_Thickness,
						.backgroundColor = Silica::GetTheme().Background_Popup,
						.child = Silica::MakeWidget<Silica::SVerticalBox>({
							.spacing = EditorTheme::SPACING_SMALL,
							.slots = {
								{ {0,0}, SilicaHelpers::MakeOptionsMenuItem("Rescan Libraries", [this]() {
									scanLibraries();
									rebuildUI();
								}) }
							}
						})
					})
				})
			});

			// -- Assemble Standard Toolbar --
			auto topBarBox = Silica::MakeWidget<Silica::SBox>({
				.padding = { EditorTheme::TOOLBAR_PADDING_X, 0.0f },
				.explicitSize = Silica::Vec2{ 0.0f, EditorTheme::TOOLBAR_HEIGHT },
				.backgroundColor = Silica::GetTheme().Surface_Tertiary,
				.child = Silica::MakeWidget<Silica::SHorizontalBox>({
					.spacing = EditorTheme::TOOLBAR_SPACING,
					.slots = {
						{ {0,0}, optionsMenu },
						{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Asset Library" })
						})},
						{ {1,0}, Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Right,
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = searchBar
						})},
						{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = m_packCountText
						})}
					}
				})
			});

			// -- Grid Container --
			m_gridContainer = Silica::MakeWidget<Silica::SBox>({.padding = { EditorTheme::PADDING_XLARGE, EditorTheme::PADDING_XLARGE } });

			auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({.child = m_gridContainer });

			m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
				.topBar = topBarBox,
				.contentArea = scrollBox
			}));

			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void AssetLibraryPanel::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;
		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void AssetLibraryPanel::rebuildUI_Internal() {
		if (!m_gridContainer || !m_packCountText) return;

		// -- Filter Packs --
		std::vector<AssetPack> filteredPacks;
		std::string queryLower = m_searchQuery;
		std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);

		for (const auto& pack : m_availablePacks) {
			std::string nameLower = pack.name;
			std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
			if (!queryLower.empty() && nameLower.find(queryLower) == std::string::npos) continue;
			filteredPacks.push_back(pack);
		}

		m_packCountText->setText((filteredPacks.size() == 1) ? "1 Pack Found" : std::to_string(filteredPacks.size()) + " Packs Found");

		Silica::WidgetPtr contentAreaWidget = nullptr;

		if (filteredPacks.empty()) {
			contentAreaWidget = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = m_searchQuery.empty() ? "No Asset Packs found in library directories." : "No packs match your search.",
					.color = Silica::GetTheme().Text_Dim
				})
			});
		}
		else {
			std::vector<Silica::WidgetPtr> cardWidgets;
			for (const AssetPack& pack : filteredPacks) {
				cardWidgets.push_back(createPackCardWidget(pack));
			}

			contentAreaWidget = Silica::MakeWidget<Silica::SWrapBox>({
				.spacing = EditorTheme::SPACING_LARGE,
				.children = cardWidgets
			});
		}

		m_gridContainer->setChild(contentAreaWidget);
	}

	void AssetLibraryPanel::importAssetPackToProject(const AssetPack& pack) {
		if (!ProjectManager::hasProject()) return;

		EditorActionQueue::push([this, pack]() {
			try {
				std::filesystem::path projectAssetsPath = ProjectManager::getProject()->getAssetsPath();
				std::filesystem::path destFolder = projectAssetsPath / "ImportedPacks" / pack.name;

				if (!std::filesystem::exists(destFolder)) {
					std::filesystem::create_directories(destFolder);
				}

				// -- Helper To Identify Valid Axion Metadata Files --
				auto getAssetTypeFromExtension = [](const std::string& ext) -> AssetType {
					if (ext == ".axmat") return AssetType::Material;
					if (ext == ".axmesh") return AssetType::Mesh;
					if (ext == ".axprefab") return AssetType::Prefab;
					if (ext == ".axtex") return AssetType::Texture2D;
					if (ext == ".axscene") return AssetType::Scene;
					if (ext == ".axanim") return AssetType::AnimationClip;
					if (ext == ".axaudio") return AssetType::AudioClip;
					if (ext == ".axpmat") return AssetType::PhysicsMaterial;
					if (ext == ".axpso") return AssetType::Pipeline;
					if (ext == ".axshader") return AssetType::Shader;
					if (ext == ".axskelmesh") return AssetType::SkeletalMesh;
					if (ext == ".axtcube") return AssetType::TextureCube;
					if (ext == ".axsky") return AssetType::Skybox;
					return AssetType::None;
				};

				// -- Pass 1 : Generate UUID Mappings (STRING BASED FOR SAFETY) --
				std::unordered_map<std::string, std::string> uuidRemap;

				for (const auto& entry : std::filesystem::recursive_directory_iterator(pack.sourcePath)) {
					if (entry.is_directory()) continue;
					std::string ext = entry.path().extension().string();

					// -- Check If Axion Asset --
					if (getAssetTypeFromExtension(ext) != AssetType::None) {
						try {
							YAML::Node node = YAML::LoadFile(entry.path().string());
							std::string oldUUIDStr = "";

							if (node["UUID"]) oldUUIDStr = node["UUID"].as<std::string>();
							else if (node["Asset"]) oldUUIDStr = node["Asset"].as<std::string>();
							else if (node["ID"]) oldUUIDStr = node["ID"].as<std::string>();

							if (!oldUUIDStr.empty() && oldUUIDStr != "0") {
								UUID newId = UUID::generate();
								uuidRemap[oldUUIDStr] = newId.toString();
							}
						}
						catch (const YAML::Exception& e) {
							AX_CORE_LOG_WARN("Asset Pack YAML parse error on {0}: {1}", entry.path().filename().string(), e.what());
						}
					}
				}

				// -- Pass 2 : Copy, Rewrite, And Register --
				auto registry = ProjectManager::getProject()->getAssetRegistry();
				std::vector<std::string> rawPathKeys = { "Source", "ShaderPath", "TextureCubePath", "FilePath", "Shader" };

				for (const auto& entry : std::filesystem::recursive_directory_iterator(pack.sourcePath)) {
					if (entry.is_directory()) continue;

					std::string filename = entry.path().filename().string();
					if (filename == "info.yaml" || filename == "thumbnail.png") continue;

					std::filesystem::path relativePath = std::filesystem::relative(entry.path(), pack.sourcePath);
					std::filesystem::path targetFilePath = destFolder / relativePath;
					std::filesystem::create_directories(targetFilePath.parent_path());

					std::string ext = entry.path().extension().string();
					AssetType type = getAssetTypeFromExtension(ext);

					if (type != AssetType::None) {
						std::ifstream in(entry.path());
						std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
						in.close();

						for (const auto& [oldStr, newStr] : uuidRemap) {
							size_t pos = 0;
							while ((pos = content.find(oldStr, pos)) != std::string::npos) {
								content.replace(pos, oldStr.length(), newStr);
								pos += newStr.length();
							}
						}

						std::ofstream out(targetFilePath);
						out << content;
						out.close();

						try {
							YAML::Node targetNode = YAML::LoadFile(targetFilePath.string());
							bool yamlNeedsResave = false;

							for (const auto& key : rawPathKeys) {
								if (targetNode[key]) {
									std::string oldSourceStr = targetNode[key].as<std::string>();
									std::string targetFileName = std::filesystem::path(oldSourceStr).filename().string();
									std::string resolvedNewSource = "";

									for (const auto& searchEntry : std::filesystem::recursive_directory_iterator(pack.sourcePath)) {
										if (searchEntry.is_regular_file() && searchEntry.path().filename().string() == targetFileName) {
											std::filesystem::path relToPack = std::filesystem::relative(searchEntry.path(), pack.sourcePath);
											std::filesystem::path finalDestPath = destFolder / relToPack;
											resolvedNewSource = PathResolver::virtualize(finalDestPath);
											break;
										}
									}

									if (!resolvedNewSource.empty()) {
										targetNode[key] = resolvedNewSource;
										yamlNeedsResave = true;
									}
									else {
										AX_CORE_LOG_WARN("Could not resolve raw file '{0}' for asset '{1}'", targetFileName, targetFilePath.filename().string());
									}
								}
							}

							if (yamlNeedsResave) {
								std::ofstream outFixed(targetFilePath);
								outFixed << targetNode;
								outFixed.close();
							}

							UUID fileUUID;
							if (targetNode["UUID"]) fileUUID = targetNode["UUID"].as<UUID>();
							else if (targetNode["Asset"]) fileUUID = targetNode["Asset"].as<UUID>();
							else if (targetNode["ID"]) fileUUID = targetNode["ID"].as<UUID>();

							if (fileUUID.isValid()) {
								AssetMetadata metadata;
								metadata.handle = fileUUID;
								metadata.type = type;
								metadata.filePath = AssetManager::getRelativeToAssets(targetFilePath);
								registry->add(metadata);
							}
						}
						catch (...) {}
					}
					else {
						std::filesystem::copy(entry.path(), targetFilePath, std::filesystem::copy_options::overwrite_existing);
					}
				}

				registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");
				AX_CORE_LOG_INFO("Successfully Deep-Imported Pack '{0}' to Project!", pack.name);
			}
			catch (const std::exception& e) {
				AX_CORE_LOG_ERROR("Failed to import asset pack: {0}", e.what());
			}
		});
	}

	void AssetLibraryPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(AssetLibraryPanel::onProjectChanged));
		dispatcher.dispatch<EditorSettingsChangedEvent>(AX_BIND_EVENT_FN(AssetLibraryPanel::onEditorSettingsChanged));
	}

	EventReply AssetLibraryPanel::onProjectChanged(ProjectChangedEvent& e) {
		m_projectIsLoaded = ProjectManager::hasProject();
		rebuildUI_Internal();
		return EventReply::unhandled();
	}

	EventReply AssetLibraryPanel::onEditorSettingsChanged(EditorSettingsChangedEvent& ev) {
		if (ev.hasChange(EditorSettingType::AssetLibraryPaths) || ev.hasChange(EditorSettingType::All)) {
			m_libraryPaths.clear();

			for (const std::string& pathStr : EditorSettings::assetLibraryPaths) {
				m_libraryPaths.push_back(std::filesystem::path(pathStr));
			}

			scanLibraries();
			rebuildUI();
		}

		return EventReply::unhandled();
	}

	void AssetLibraryPanel::setLibraryDirectories(const std::vector<std::filesystem::path>& paths) {
		m_libraryPaths = paths;
		scanLibraries();
		rebuildUI();
	}

	Silica::WidgetPtr AssetLibraryPanel::createPackCardWidget(const AssetPack& pack) {
		Silica::WidgetPtr thumbnail;
		if (pack.thumbnailID != 0) {
			thumbnail = Silica::MakeWidget<Silica::SImage>({ .textureID = pack.thumbnailID });
		}
		else {
			thumbnail = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "No Image", .color = Silica::GetTheme().Text_Dim })
				});
		}

		return Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{ CARD_WIDTH, CARD_HEIGHT },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.child = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = EditorTheme::SPACING_MEDIUM,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2{ CARD_WIDTH, THUMBNAIL_HEIGHT },
						.backgroundColor = Silica::GetTheme().Surface_Secondary,
						.child = thumbnail
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SBox>({
						.padding = { EditorTheme::PADDING_MEDIUM, 0.0f },
						.explicitSize = Silica::Vec2{ CARD_WIDTH - (EditorTheme::PADDING_MEDIUM * 2.0f), TITLE_HEIGHT },
						.child = Silica::MakeWidget<Silica::STextBlock>({
							.text = pack.name,
							.truncateWidth = CARD_WIDTH - (EditorTheme::PADDING_MEDIUM * 2.0f)
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SBox>({
						.padding = { EditorTheme::PADDING_MEDIUM, 0.0f },
						.explicitSize = Silica::Vec2{ CARD_WIDTH - (EditorTheme::PADDING_MEDIUM * 2.0f), DESC_HEIGHT },
						.child = Silica::MakeWidget<Silica::SWrappedTextBlock>({
							.text = pack.description,
							.wrapWidth = CARD_WIDTH - (EditorTheme::PADDING_MEDIUM * 2.0f),
							.maxLines = DESC_MAX_LINES,
							.color = Silica::GetTheme().Text_Dim
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SBox>({
						.padding = { EditorTheme::PADDING_MEDIUM, EditorTheme::PADDING_MEDIUM },
						.child = Silica::MakeWidget<Silica::SButton>({
							.padding = { 0.0f, EditorTheme::BUTTON_PADDING_Y },
							.enabled = m_projectIsLoaded,
							.color = Silica::GetTheme().Accent_Primary,
							.onClick = [this, pack]() {
								importAssetPackToProject(pack);
								return Silica::EventReply::handled();
							},
							.child = Silica::MakeWidget<Silica::SAlign>({
								.horizontalAlign = Silica::HorizontalAlign::Center,
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import Pack"})
							})
						})
					})}
				}
			})
		});
	}

}
