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

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

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
