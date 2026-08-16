#include "AssetLibraryPanel.h"

#include <algorithm>
#include <cctype>

#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/YAMLHelper.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/Theme.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SImage.h"
#include "AxionStudio/Vendor/Silica/include/SWrapBox.h"
#include "AxionStudio/Vendor/Silica/include/SWrappedTextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

namespace Axion {

	AssetLibraryPanel::AssetLibraryPanel() {
		std::filesystem::path defaultLib = std::filesystem::current_path() / "AxionStudio" / "Resources" / "DefaultAssets";
		if (!std::filesystem::exists(defaultLib)) std::filesystem::create_directories(defaultLib);

		addLibraryDirectory(defaultLib);
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
						if (info["Name"]) {
							pack.name = info["Name"].as<std::string>();
						}
						if (info["Description"]) {
							pack.description = info["Description"].as<std::string>();
						}
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
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({ .borderThickness = Silica::GetTheme().Border_Thickness });

			// -- Pack Count --
			m_packCountText = Silica::MakeWidget<Silica::STextBlock>({
				.text = "0 Packs Found",
				.color = Silica::GetTheme().Text_Dim
			});

			// -- Search Bar --
			auto searchBar = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{ 250.0f, 0.0f },
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = m_searchQuery,
					.hintText = "Search Packs...",
					.onTextChanged = [this](const std::string& val) {
						m_searchQuery = val;
						rebuildUI();
					}
				})
			});

			// -- Assemble Top Bar --
			auto topBarBox = Silica::MakeWidget<Silica::SBox>({
				.padding = { 8.0f, 5.0f },
				.backgroundColor = Silica::GetTheme().Surface_Tertiary,
				.child = Silica::MakeWidget<Silica::SHorizontalBox>({
					.spacing = 20.0f,
					.slots = {
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
			m_gridContainer = Silica::MakeWidget<Silica::SBox>({.padding = { 20.0f, 20.0f } });

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

			if (!queryLower.empty() && nameLower.find(queryLower) == std::string::npos) {
				continue;
			}
			filteredPacks.push_back(pack);
		}

		// -- Update Count Text --
		m_packCountText->setText((filteredPacks.size() == 1) ? "1 Pack Found" : std::to_string(filteredPacks.size()) + " Packs Found");

		// -- Generate Grid Content --
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
				Silica::WidgetPtr thumbnail;
				if (pack.thumbnailID != 0) {
					thumbnail = Silica::MakeWidget<Silica::SImage>({ .textureID = pack.thumbnailID });
				}
				else {
					thumbnail = Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Center,
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({
							.text = "No Image",
							.color = Silica::GetTheme().Text_Dim
						})
						});
				}

				auto packCard = Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2{ 220.0f, 300.0f },
					.borderThickness = Silica::GetTheme().Border_Thickness,
					.child = Silica::MakeWidget<Silica::SVerticalBox>({
						.spacing = 8.0f,
						.slots = {
							{ {0,0}, Silica::MakeWidget<Silica::SBox>({
								.explicitSize = Silica::Vec2{ 220.0f, 140.0f },
								.backgroundColor = Silica::GetTheme().Surface_Secondary,
								.child = thumbnail
							})},
							{ {0,0}, Silica::MakeWidget<Silica::SBox>({
								.padding = {8.0f, 0.0f},
								.explicitSize = Silica::Vec2{ 204.0f, 20.0f },
								.child = Silica::MakeWidget<Silica::STextBlock>({
									.text = pack.name,
									.truncateWidth = 204.0f
								})
							})},
							{ {0,0}, Silica::MakeWidget<Silica::SBox>({
								.padding = {8.0f, 0.0f},
								.explicitSize = Silica::Vec2{ 204.0f, 60.0f },
								.child = Silica::MakeWidget<Silica::SWrappedTextBlock>({
									.text = pack.description,
									.wrapWidth = 204.0f,
									.maxLines = 3,
									.color = Silica::GetTheme().Text_Dim
								})
							})},
							{ {0,0}, Silica::MakeWidget<Silica::SBox>({
								.padding = {8.0f, 12.0f},
								.child = Silica::MakeWidget<Silica::SButton>({
									.padding = { 0.0f, 6.0f },
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
				cardWidgets.push_back(packCard);
			}

			contentAreaWidget = Silica::MakeWidget<Silica::SWrapBox>({
				.spacing = 15.0f,
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
					return AssetType::None;
				};

				// -- Pass 1 : Generate UUID Mappings --
				std::unordered_map<UUID, UUID> uuidRemap;

				for (const auto& entry : std::filesystem::recursive_directory_iterator(pack.sourcePath)) {
					if (entry.is_directory()) continue;
					std::string ext = entry.path().extension().string();

					// -- Check If Axion Asset --
					if (getAssetTypeFromExtension(ext) != AssetType::None) {
						try {
							YAML::Node node = YAML::LoadFile(entry.path().string());
							UUID oldUUID;
							oldUUID.invalidate();

							if (node["UUID"]) oldUUID = node["UUID"].as<UUID>();
							else if (node["Asset"]) oldUUID = node["Asset"].as<UUID>();
							else if (node["ID"]) oldUUID = node["ID"].as<UUID>();

							if (oldUUID.isValid()) {
								uuidRemap[oldUUID] = UUID::generate();
							}
						}
						catch (const YAML::Exception& e) {
							AX_CORE_LOG_WARN("Asset Pack YAML parse error on {0}: {1}", entry.path().filename().string(), e.what());
						}
					}
				}


				// -- Pass 2 : Copy, Rewrite, And Register --
				auto registry = ProjectManager::getProject()->getAssetRegistry();

				for (const auto& entry : std::filesystem::recursive_directory_iterator(pack.sourcePath)) {
					if (entry.is_directory()) continue;

					std::string filename = entry.path().filename().string();
					if (filename == "info.yaml" || filename == "thumbnail.png") continue;

					// -- Replicate Folder Structure --
					std::filesystem::path relativePath = std::filesystem::relative(entry.path(), pack.sourcePath);
					std::filesystem::path targetFilePath = destFolder / relativePath;
					std::filesystem::create_directories(targetFilePath.parent_path());

					std::string ext = entry.path().extension().string();
					AssetType type = getAssetTypeFromExtension(ext);

					if (type != AssetType::None) {
						std::ifstream in(entry.path());
						std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
						in.close();

						// -- Safely Replace All Internal UUID References --
						for (const auto& [oldId, newId] : uuidRemap) {
							std::string oldStr = oldId.toString();
							std::string newStr = newId.toString();
							size_t pos = 0;
							while ((pos = content.find(oldStr, pos)) != std::string::npos) {
								content.replace(pos, oldStr.length(), newStr);
								pos += newStr.length();
							}
						}

						std::ofstream out(targetFilePath);
						out << content;
						out.close();

						// -- Inject Into Asset Registry And Fix Source Paths --
						try {
							YAML::Node targetNode = YAML::LoadFile(targetFilePath.string());
							bool yamlNeedsResave = false;

							// -- Smart Path Resolver --
							if (targetNode["Source"]) {
								std::string oldSource = targetNode["Source"].as<std::string>();
								std::string targetFileName = std::filesystem::path(oldSource).filename().string();
								std::filesystem::path resolvedNewSource = "";

								// -- Root-Relative --
								if (std::filesystem::exists(destFolder / oldSource)) {
									resolvedNewSource = std::filesystem::path("ImportedPacks") / pack.name / oldSource;
								}

								// -- Sibling-Relative --
								else if (std::filesystem::exists(targetFilePath.parent_path() / oldSource)) {
									resolvedNewSource = std::filesystem::relative(targetFilePath.parent_path() / oldSource, projectAssetsPath);
								}

								// -- Deep Search --
								else {
									for (const auto& searchEntry : std::filesystem::recursive_directory_iterator(destFolder)) {
										if (searchEntry.path().filename().string() == targetFileName) {
											resolvedNewSource = std::filesystem::relative(searchEntry.path(), projectAssetsPath);
											break;
										}
									}
								}

								// -- Update YAML --
								if (!resolvedNewSource.empty()) {
									targetNode["Source"] = resolvedNewSource.generic_string();
									yamlNeedsResave = true;
								}
								else {
									AX_CORE_LOG_WARN("Could not resolve Source file '{}' for asset '{}'", targetFileName, targetFilePath.filename().string());
								}
							}

							if (yamlNeedsResave) {
								std::ofstream outFixed(targetFilePath);
								outFixed << targetNode;
								outFixed.close();
							}

							UUID fileUUID;
							fileUUID.invalidate();

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
						// -- Copy Raw Files --
						std::filesystem::copy(entry.path(), targetFilePath, std::filesystem::copy_options::overwrite_existing);
					}
				}

				// --- Finalize ---
				registry->serialize(ProjectManager::getProject()->getProjectPath() / "AssetRegistry.yaml");

				AX_CORE_LOG_INFO("Successfully Deep-Imported Pack '{0}' to Project!", pack.name);
			}
			catch (const std::exception& e) {
				AX_CORE_LOG_ERROR("Failed to import asset pack: {0}", e.what());
			}
		});
	}

	void AssetLibraryPanel::onEvent(Event& e) {
		if (e.getEventType() == EventType::ProjectChanged) {
			m_projectIsLoaded = ProjectManager::hasProject();
			rebuildUI_Internal();
		}
	}

	void AssetLibraryPanel::setLibraryDirectories(const std::vector<std::filesystem::path>& paths) {
		m_libraryPaths = paths;
		scanLibraries();
		rebuildUI();
	}

}
