#include "studiopch.h"
#include "SceneSettingsPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SSliderFloat.h>
#include <Silica/include/SColorField.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SInputFieldVec3Float.h>
#include <Silica/include/SWrappedTextBlock.h>
#include <Silica/include/SScissorBox.h>
#include <Silica/include/SImage.h>
#include <Silica/include/SSpacer.h>
#include <Silica/include/SInputFieldFloat.h>

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/NavMeshParser.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"
#include "AxionStudio/Source/ui/EditorTheme.h"

namespace {
	constexpr float DROP_ZONE_PADDING = 4.0f;
}

namespace Axion {

	Silica::WidgetPtr SceneSettingsPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({.hasBorder = true });
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void SceneSettingsPanel::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void SceneSettingsPanel::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		// -- Empty States --
		if (!ProjectManager::hasProject()) {
			m_uiRoot->setChild(SilicaHelpers::MakeEmptyState("No Project Loaded.\n\nPlease load or create a project from the top menu bar to view scene settings."));
			return;
		}

		if (!m_activeScene) {
			m_uiRoot->setChild(SilicaHelpers::MakeEmptyState("No Scene Loaded."));
			return;
		}

		// -- Options Menu --
		auto optionsMenu = Silica::MakeWidget<Silica::SAlign>({
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = Silica::MakeWidget<Silica::SMenuAnchor>({
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
					.hasBorder = true,
					.backgroundColor = Silica::GetTheme().Background_Popup,
					.child = Silica::MakeWidget<Silica::SVerticalBox>({
						.spacing = EditorTheme::SPACING_SMALL,
						.slots = {
							{ {0,0}, SilicaHelpers::MakeOptionsMenuItem("Reset Gravity", [this]() {
								m_activeScene->setGravity(Vec3(0.0f, -9.81f, 0.0f));
								rebuildUI();
							}) }
						}
					})
				})
			})
		});

		// -- Toolbar --
		auto topBarBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::TOOLBAR_PADDING_X, 0.0f },
			.explicitSize = Silica::Vec2{ 0.0f, EditorTheme::TOOLBAR_HEIGHT },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::TOOLBAR_SPACING,
				.slots = {
					{ {0, 0}, optionsMenu },
					{ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { EditorTheme::SPACING_LARGE, 0.0f } }) },
					{ {0, 0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Scene Settings" })
					}) }
				}
			})
		});

		// -- Build Scrollable Content Area --
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE });

		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakeHeader("General") });
		auto generalContent = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE });

		auto nameInput = Silica::MakeWidget<Silica::SEditableText>({
			.initialText = m_activeScene->getTitle(),
			.onTextCommitted = [this](const std::string& val) {
				m_activeScene->setTitle(val);
			}
		});
		generalContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Scene Name:", nameInput) });

		auto generalDummyZone = SilicaHelpers::MakeAssetDropZone("", [](const std::filesystem::path&) {}, generalContent, { DROP_ZONE_PADDING, DROP_ZONE_PADDING });
		contentBox->addSlot({ {0, 0}, generalDummyZone });

		contentBox->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { 0.0f, EditorTheme::SPACING_LARGE }}) });

		// ----- SKYBOX -----
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakeHeader("Skybox") });
		auto skyboxContent = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE });

		if (m_activeScene->hasSkybox()) {
			std::filesystem::path skyPath = AssetManager::getAssetFilePath<Skybox>(m_activeScene->getSkyboxHandle());
			std::filesystem::path skyRel = AssetManager::getRelativeToAssets(skyPath);

			skyboxContent->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("File Name:", skyPath.filename().string()) });
			skyboxContent->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("File Path:", skyRel.generic_string()) });

			auto changeBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.onClick = [this]() {
					std::filesystem::path skyDir = ProjectManager::getProject()->getAssetsPath() / "Skybox";
					std::filesystem::path absolutePath = std::filesystem::exists(skyDir) ?
						FileDialogs::openFile({ {"Axion Skybox Asset", "*.axsky"} }, skyDir) :
						FileDialogs::openFile({ {"Axion Skybox Asset", "*.axsky"} }, ProjectManager::getProject()->getAssetsPath());

					if (!absolutePath.empty()) {
						UUID assetUUID = AssetManager::getAssetUUID(absolutePath);
						if (assetUUID.isValid()) {
							AssetHandle<Skybox> handle = AssetManager::load<Skybox>(assetUUID);
							m_activeScene->setSkybox(handle);
							rebuildUI();
						}
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Change Skybox"})
			});

			auto removeBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.color = Silica::GetTheme().Accent_Danger,
				.onClick = [this]() {
					m_activeScene->removeSkybox();
					rebuildUI();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Remove"})
			});

			auto options = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::SPACING_MEDIUM,
				.slots = {
					{ {0,0}, changeBtn },
					{ {0,0}, removeBtn }
				}
			});

			skyboxContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Options:", options) });
		}
		else {
			auto loadBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.onClick = [this]() {
					std::filesystem::path skyDir = ProjectManager::getProject()->getAssetsPath() / "Skybox";
					std::filesystem::path absolutePath = std::filesystem::exists(skyDir) ?
						FileDialogs::openFile({ {"Axion Skybox Asset", "*.axsky"} }, skyDir) :
						FileDialogs::openFile({ {"Axion Skybox Asset", "*.axsky"} }, ProjectManager::getProject()->getAssetsPath());

					if (!absolutePath.empty()) {
						UUID assetUUID = AssetManager::getAssetUUID(absolutePath);
						if (assetUUID.isValid()) {
							AssetHandle<Skybox> handle = AssetManager::load<Skybox>(assetUUID);
							m_activeScene->setSkybox(handle);
							rebuildUI();
						}
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Select Skybox" })
			});

			skyboxContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Options:", loadBtn) });
		}

		auto skyboxDropZone = SilicaHelpers::MakeAssetDropZone(".axsky", [this](const std::filesystem::path& droppedPath) {
			EditorActionQueue::push([this, droppedPath]() {
				if (!m_activeScene) return;
				UUID assetUUID = AssetManager::getAssetUUID(droppedPath);
				if (assetUUID.isValid()) {
					AssetHandle<Skybox> handle = AssetManager::load<Skybox>(assetUUID);
					m_activeScene->setSkybox(handle);
					SceneModifiedEvent ev(SceneModificationType::SkyboxChanged);
					m_eventCallback(ev);
					rebuildUI();
				}
				else {
					AX_CORE_LOG_WARN("Attempted to drop invalid Skybox asset!");
				}
			});
			
		}, skyboxContent, { DROP_ZONE_PADDING, DROP_ZONE_PADDING });

		contentBox->addSlot({ {0, 0}, skyboxDropZone });
		contentBox->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { 0.0f, EditorTheme::SPACING_LARGE }}) });

		// ----- PHYSICS -----
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakeHeader("Physics") });

		auto physicsContent = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE });

		Vec3 currentGravity = m_activeScene->getGravity();
		auto gravityInput = Silica::MakeWidget<Silica::SInputFieldVec3Float>({
			.label = "",
			.initialValue = Silica::Vec3(currentGravity.x, currentGravity.y, currentGravity.z),
			.onValueChanged = [this](Silica::Vec3 val) {
				m_activeScene->setGravity(Vec3(val.x, val.y, val.z));
			}
		});
		physicsContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Global Gravity:", gravityInput) });

		auto physicsDummyZone = SilicaHelpers::MakeAssetDropZone("", [](const std::filesystem::path&) {}, physicsContent, { DROP_ZONE_PADDING, DROP_ZONE_PADDING });
		contentBox->addSlot({ {0, 0}, physicsDummyZone });

		contentBox->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { 0.0f, EditorTheme::SPACING_LARGE }}) });

		// ----- GRAPHICS -----
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakeHeader("Graphics") });

		auto graphicsContent = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE });

		Vec4 sceneColor = m_activeScene->getAmbientColor();
		auto ambientColorInput = Silica::MakeWidget<Silica::SColorField>({
			.initialColor = SilicaHelpers::MakeSilicaColor(sceneColor),
			.onColorChanged = [this](Silica::Color c) {
				m_activeScene->setAmbientColor(SilicaHelpers::MakeAxionColor(c));
			}
		});
		graphicsContent->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Ambient Color:", ambientColorInput) });

		auto graphicsDummyZone = SilicaHelpers::MakeAssetDropZone("", [](const std::filesystem::path&) {}, graphicsContent, { DROP_ZONE_PADDING, DROP_ZONE_PADDING });
		contentBox->addSlot({ {0, 0}, graphicsDummyZone });

		contentBox->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { 0.0f, EditorTheme::SPACING_LARGE }}) });

		// ----- PATHFINDING -----
		contentBox->addSlot({ {0, 0}, SilicaHelpers::MakeHeader("Pathfinding Profiles") });
		auto pathfindingContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = EditorTheme::SPACING_LARGE });

		auto& profiles = m_activeScene->getNavMeshProfiles();

		for (size_t i = 0; i < profiles.size(); ++i) {
			auto& profile = profiles[i];
			auto profileBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = EditorTheme::SPACING_MEDIUM });

			// -- Name & Delete Row --
			auto nameInput = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = profile.name,
				.onTextCommitted = [this, i](const std::string& val) {
					m_activeScene->getNavMeshProfiles()[i].name = val;
				}
				});

			auto deleteBtn = Silica::MakeWidget<Silica::SButton>({
				.color = Silica::GetTheme().Accent_Danger,
				.onClick = [this, i]() {
					m_activeScene->getNavMeshProfiles().erase(m_activeScene->getNavMeshProfiles().begin() + i);
					rebuildUI();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X"})
				});

			auto headerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::SPACING_MEDIUM,
				.slots = { { {1, 0}, nameInput }, { {0, 0}, deleteBtn } }
				});
			profileBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Profile Name:", headerRow) });

			// -- Active Asset Display --
			if (profile.handle.isValid()) {
				std::filesystem::path nmPath = AssetManager::getAssetFilePath<NavMesh>(profile.handle);
				profileBox->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("Active NavMesh:", nmPath.filename().string()) });

				auto removeNmBtn = Silica::MakeWidget<Silica::SButton>({
					.color = Silica::GetTheme().Accent_Danger,
					.onClick = [this, i]() {
						m_activeScene->getNavMeshProfiles()[i].handle.invalidate();
						rebuildUI();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Clear NavMesh"})
					});
				profileBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("", removeNmBtn) });
			}
			else {
				profileBox->addSlot({ {0, 0}, SilicaHelpers::MakeDetailRow("Active NavMesh:", "None") });
			}

			// -- Settings Inputs --
			auto hInput = Silica::MakeWidget<Silica::SInputFieldFloat>({
				.initialValue = profile.settings.agentHeight,
				.onValueChanged = [this, i](float val) { m_activeScene->getNavMeshProfiles()[i].settings.agentHeight = std::max(0.1f, val); }
			});
			profileBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Walkable Height:", hInput) });

			auto rInput = Silica::MakeWidget<Silica::SInputFieldFloat>({
				.initialValue = profile.settings.agentRadius,
				.onValueChanged = [this, i](float val) { m_activeScene->getNavMeshProfiles()[i].settings.agentRadius = std::max(0.1f, val); }
			});
			profileBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Agent Radius:", rInput) });

			auto cInput = Silica::MakeWidget<Silica::SInputFieldFloat>({
				.initialValue = profile.settings.agentMaxClimb,
				.onValueChanged = [this, i](float val) { m_activeScene->getNavMeshProfiles()[i].settings.agentMaxClimb = std::max(0.0f, val); }
			});
			profileBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Max Climb:", cInput) });

			auto sInput = Silica::MakeWidget<Silica::SInputFieldFloat>({
				.initialValue = profile.settings.agentMaxSlope,
				.onValueChanged = [this, i](float val) { m_activeScene->getNavMeshProfiles()[i].settings.agentMaxSlope = std::clamp(val, 0.0f, 90.0f); }
			});
			profileBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Max Slope (Deg):", sInput) });

			auto csInput = Silica::MakeWidget<Silica::SInputFieldFloat>({
				.initialValue = profile.settings.cellSize,
				.onValueChanged = [this, i](float val) { m_activeScene->getNavMeshProfiles()[i].settings.cellSize = std::max(0.05f, val); }
			});
			profileBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Cell Size:", csInput) });

			auto chInput = Silica::MakeWidget<Silica::SInputFieldFloat>({
				.initialValue = profile.settings.cellHeight,
				.onValueChanged = [this, i](float val) { m_activeScene->getNavMeshProfiles()[i].settings.cellHeight = std::max(0.05f, val); }
			});
			profileBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Cell Height:", chInput) });

			// -- Bake Button --
			auto bakeBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.onClick = [this, i]() {
					auto& curProfile = m_activeScene->getNavMeshProfiles()[i];
					Ref<NavMesh> newMesh = NavMeshSystem::bakeScene(m_activeScene.get(), curProfile.settings);

					if (newMesh) {
						// CASE 1: Update an EXISTING NavMesh (No File Dialog, No New UUID)
						if (curProfile.handle.isValid()) {
							std::filesystem::path nmPath = AssetManager::getAssetFilePath<NavMesh>(curProfile.handle);
							if (!nmPath.empty()) {
								std::filesystem::path rawSourcePath = AssetManager::getAbsolute(nmPath);
								rawSourcePath.replace_extension(".axnav");

								// Overwrite the binary data on disk
								std::ofstream outStream(rawSourcePath, std::ios::out | std::ios::binary);
								NavMeshSystem::serializeToStream(newMesh, outStream);
								outStream.close();

								// Update live RAM
								AssetManager::storage<NavMesh>().assets[curProfile.handle.uuid] = newMesh;
								AX_CORE_LOG_INFO("Successfully updated existing NavMesh for profile: {}", curProfile.name);
								rebuildUI();
								return Silica::EventReply::handled();
							}
						}

						// CASE 2: Bake a completely NEW NavMesh
						std::filesystem::path defaultPath = ProjectManager::getProject()->getAssetsPath();
						std::filesystem::path savePath = FileDialogs::saveFile({ {"Axion NavMesh", "*.axnm"} }, defaultPath);

						if (!savePath.empty()) {
							std::filesystem::path rawSourcePath = savePath;
							rawSourcePath.replace_extension(".axnav");

							// Save binary
							std::ofstream outStream(rawSourcePath, std::ios::out | std::ios::binary);
							NavMeshSystem::serializeToStream(newMesh, outStream);
							outStream.close();

							// Save YAML metadata
							UUID newUUID = UUID::generate();
							AAP::NavMeshAssetData data;
							data.uuid = newUUID;
							data.name = savePath.stem().string();
							// FIX: Convert Absolute to Relative Path!
							data.sourcePath = rawSourcePath;
							AAP::NavMeshParser::createTextFile(data, savePath);

							// Register new asset
							AssetMetadata metadata;
							metadata.handle = newUUID;
							metadata.type = AssetType::NavMesh;
							metadata.filePath = AssetManager::getRelativeToAssets(savePath);

							ProjectManager::getProject()->getAssetRegistry()->add(metadata);
							ProjectManager::getProject()->getAssetRegistry()->serialize(ProjectManager::getProjectFilePath().parent_path() / "AssetRegistry.yaml");

							// Apply to RAM and scene
							AssetManager::storage<NavMesh>().assets[newUUID] = newMesh;
							AssetManager::storage<NavMesh>().handleToPath[newUUID] = savePath;

							m_activeScene->getNavMeshProfiles()[i].handle = newUUID;
							AX_CORE_LOG_INFO("Successfully baked NEW NavMesh for profile: {}", curProfile.name);
							rebuildUI();
						}
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::SAlign>({
					.horizontalAlign = Silica::HorizontalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Bake NavMesh" })
				})
			});
			profileBox->addSlot({ {0, 0}, SilicaHelpers::MakePropertyRow("Actions:", bakeBtn) });

			// -- Drop Zone --
			auto dropZone = SilicaHelpers::MakeAssetDropZone(".axnm", [this, i](const std::filesystem::path& droppedPath) {
				EditorActionQueue::push([this, i, droppedPath]() {
					if (!m_activeScene) return;
					UUID assetUUID = AssetManager::getAssetUUID(droppedPath);
					if (assetUUID.isValid()) {
						m_activeScene->getNavMeshProfiles()[i].handle = AssetManager::load<NavMesh>(assetUUID);
						rebuildUI();
					}
				});
			}, profileBox, { DROP_ZONE_PADDING, DROP_ZONE_PADDING });

			pathfindingContent->addSlot({ {0, 0}, dropZone });
			pathfindingContent->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSpacer>({.size = { 0.0f, EditorTheme::SPACING_LARGE }}) });
		}

		// -- Add Profile Button --
		auto addProfileBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.onClick = [this]() {
				SceneNavMeshProfile newProfile;
				newProfile.name = "New Profile";
				m_activeScene->getNavMeshProfiles().push_back(newProfile);
				rebuildUI();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "+ Add NavMesh Profile" })
			})
		});

		pathfindingContent->addSlot({ {0, 0}, addProfileBtn });
		contentBox->addSlot({ {0, 0}, pathfindingContent });



		// -- Final Layout Assembly --
		auto paddedContent = Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::PADDING_LARGE, EditorTheme::PADDING_LARGE },
			.child = contentBox
		});

		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({.child = paddedContent });

		auto borderLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = topBarBox,
			.contentArea = scrollBox
		});

		m_uiRoot->setChild(borderLayout);
	}

	void SceneSettingsPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(SceneSettingsPanel::onSceneChanged));
		dispatcher.dispatch<SceneModifiedEvent>(AX_BIND_EVENT_FN(SceneSettingsPanel::onSceneModified));
	}

	EventReply SceneSettingsPanel::onSceneChanged(SceneChangedEvent& ev) {
		setScene(SceneManager::getScene());
		return EventReply::unhandled();
	}

	EventReply SceneSettingsPanel::onSceneModified(SceneModifiedEvent& ev) {
		rebuildUI_Internal();
		return EventReply::unhandled();
	}

	void SceneSettingsPanel::setScene(const Shared<Scene>& scene) {
		m_activeScene = scene;
		rebuildUI();
	}

}
