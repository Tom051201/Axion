#include "studiopch.h"
#include "SceneOverviewPanel.h"

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

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	Silica::WidgetPtr SceneOverviewPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.onDragOver = [](const Silica::DragDropPayload& payload) {
					if (payload.type == "AssetPath") {
						auto path = std::any_cast<std::filesystem::path>(payload.data);
						if (path.extension() == ".axsky") {
							return Silica::EventReply::handled();
						}
					}
					return Silica::EventReply::unhandled();
				},
				.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
					if (payload.type == "AssetPath" && m_activeScene) {
						auto path = std::any_cast<std::filesystem::path>(payload.data);

						if (path.extension() == ".axsky") {
							EditorActionQueue::push([this, path]() mutable {
								UUID assetUUID = AssetManager::getAssetUUID(path);
								if (assetUUID.isValid()) {
									AssetHandle<Skybox> handle = AssetManager::load<Skybox>(assetUUID);
									m_activeScene->setSkybox(handle);

									SceneModifiedEvent ev(SceneModificationType::SkyboxChanged);
									m_eventCallback(ev);
								}
								else {
									AX_CORE_LOG_WARN("Attempted to drop invalid Skybox asset!");
								}
							});
							return Silica::EventReply::handled();
						}
					}
					return Silica::EventReply::unhandled();
				}
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void SceneOverviewPanel::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void SceneOverviewPanel::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		// -- No project loaded --
		if (!ProjectManager::hasProject()) {
			m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "No Project Loaded.\nPlease load or create a project first."
				})
			}));
			return;
		}

		// -- No scene loaded --
		if (!m_activeScene) {
			m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "No Scene Loaded."
				})
			}));
			return;
		}

		// -- Toolbar --
		auto titleInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::GetTheme().Background_Input,
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_activeScene->getTitle(),
				.onTextCommitted = [this](const std::string& newText) {
					m_activeScene->setTitle(newText);
				}
			})
		});

		auto topBarBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { 10.0f, 10.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({
							.text = "Scene:",
							.color = Silica::GetTheme().Text_Dim
						})
					})},
					{ {1, 0}, titleInput }
				}
			})
		});

		// -- Build Scrollable Content Area --
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 10.0f });


		// -- Helper Funtions --
		auto MakePropertyRow = [&](const std::string& label, Silica::WidgetPtr valueWidget) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(120.0f, 0.0f),
						.backgroundColor = Silica::Color::transparent(),
						.child = Silica::MakeWidget<Silica::SAlign>({
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = label })
						})
					})},
					{ {1, 0}, valueWidget }
				}
			});
		};


		// -- Skybox --
		Silica::WidgetPtr skyboxContent;

		if (m_activeScene->hasSkybox()) {
			std::filesystem::path skyPath = AssetManager::getAssetFilePath<Skybox>(m_activeScene->getSkyboxHandle());
			std::filesystem::path skyRel = AssetManager::getRelativeToAssets(skyPath);

			auto btnRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.onClick = [this]() {
							std::filesystem::path skyDir = ProjectManager::getProject()->getAssetsPath() / "skybox";
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
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.color = Silica::GetTheme().Accent_Danger,
						.onClick = [this]() {
							m_activeScene->removeSkybox();
							rebuildUI();
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Remove"})
					})}
				}
			});

			skyboxContent = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 4.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({ .text = skyPath.stem().string() })},
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
						.text = skyRel.string(),
						.color = Silica::GetTheme().Text_Dim
					})},
					{ {0,0}, btnRow }
				}
			});
		}
		else {
			skyboxContent = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
						.text = "No Skybox Loaded",
						.color = Silica::GetTheme().Text_Dim
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.onClick = [this]() {
							std::filesystem::path skyDir = ProjectManager::getProject()->getAssetsPath() / "skybox";
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
					})}
				}
			});
		}
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Skybox", skyboxContent) });


		// -- Global Gravity --
		Vec3 currentGravity = m_activeScene->getGravity();
		auto gravityInput = Silica::MakeWidget<Silica::SInputFieldVec3Float>({
			.label = "",
			.initialValue = Silica::Vec3(currentGravity.x, currentGravity.y, currentGravity.z),
			.onValueChanged = [this](Silica::Vec3 val) {
				m_activeScene->setGravity(Vec3(val.x, val.y, val.z));
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Global Gravity", gravityInput) });


		// -- Ambient Color --
		Vec4 sceneColor = m_activeScene->getAmbientColor();
		auto ambientColorInput = Silica::MakeWidget<Silica::SColorField>({
			.initialColor = Silica::Color(
				(uint8_t)(std::clamp(sceneColor.x, 0.0f, 1.0f) * 255.0f),
				(uint8_t)(std::clamp(sceneColor.y, 0.0f, 1.0f) * 255.0f),
				(uint8_t)(std::clamp(sceneColor.z, 0.0f, 1.0f) * 255.0f),
				(uint8_t)(std::clamp(sceneColor.w, 0.0f, 1.0f) * 255.0f)
			),
			.onColorChanged = [this](Silica::Color c) {
				m_activeScene->setAmbientColor(Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f));
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Ambient Color", ambientColorInput) });


		// -- Final Layout Assembly --
		auto paddedContent = Silica::MakeWidget<Silica::SBox>({
			.padding = { 10.0f, 10.0f },
			.child = contentBox
		});

		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({
			.child = paddedContent
		});

		auto borderLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = topBarBox,
			.contentArea = scrollBox
		});

		m_uiRoot->setChild(borderLayout);
	}

	void SceneOverviewPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(SceneOverviewPanel::onSceneChanged));
		dispatcher.dispatch<SceneModifiedEvent>(AX_BIND_EVENT_FN(SceneOverviewPanel::onSceneModified));
	}

	EventReply SceneOverviewPanel::onSceneChanged(SceneChangedEvent& ev) {
		setScene(SceneManager::getScene());
		return EventReply::unhandled();
	}

	EventReply SceneOverviewPanel::onSceneModified(SceneModifiedEvent& ev) {
		rebuildUI_Internal();
		return EventReply::unhandled();
	}


	void SceneOverviewPanel::setScene(const Shared<Scene>& scene) {
		m_activeScene = scene;
		rebuildUI();
	}

}
