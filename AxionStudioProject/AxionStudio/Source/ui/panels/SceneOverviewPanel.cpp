#include "SceneOverviewPanel.h"

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SSliderFloat.h"
#include "AxionStudio/Vendor/Silica/include/SColorPicker.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"
#include "AxionStudio/Vendor/Silica/include/SVector3FloatInput.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	Silica::WidgetPtr SceneOverviewPanel::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(25, 25, 25, 255)
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
					.text = "No Project Loaded.\nPlease load or create a project first.",
					.font = m_font
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
					.text = "No Scene Loaded.",
					.font = m_font
				})
			}));
			return;
		}

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
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = label, .font = m_font})
						})
					})},
					{ {1, 0}, valueWidget }
				}
			});
		};

		auto MakeColorField = [&](const Vec4& vecColor, auto onColorChanged) {
			Silica::Color initialColor = Silica::Color(
				(uint8_t)(std::clamp(vecColor.x, 0.0f, 1.0f) * 255.0f),
				(uint8_t)(std::clamp(vecColor.y, 0.0f, 1.0f) * 255.0f),
				(uint8_t)(std::clamp(vecColor.z, 0.0f, 1.0f) * 255.0f),
				(uint8_t)(std::clamp(vecColor.w, 0.0f, 1.0f) * 255.0f)
			);

			return Silica::MakeWidget<Silica::SMenuAnchor>({
				.openOnHover = false,
				.anchorContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { 2.0f, 2.0f },
					.backgroundColor = Silica::Color(45, 45, 45, 255),
					.child = Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(100.0f, 24.0f),
						.backgroundColor = initialColor
					})
				}),
				.menuContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 10.0f },
					.backgroundColor = Silica::Color(45, 45, 45, 255),
					.child = Silica::MakeWidget<Silica::SColorPicker>({
						.initialColor = initialColor,
						.onColorChanged = onColorChanged
					})
				})
			});
		};


		// -- Scene Title --
		auto titleInput = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(35, 35, 35, 255),
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.initialText = m_activeScene->getTitle(),
				.font = m_font,
				.onTextCommitted = [this](const std::string& newText) {
					m_activeScene->setTitle(newText);
				}
			})
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Title", titleInput) });


		// -- Skybox --
		Silica::WidgetPtr skyboxContent;

		if (m_activeScene->hasSkybox()) {
			std::filesystem::path skyPath = AssetManager::getAssetFilePath<Skybox>(m_activeScene->getSkyboxHandle());
			std::filesystem::path skyRel = AssetManager::getRelativeToAssets(skyPath);

			auto btnRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f }, .color = Silica::Color(50, 50, 50, 255),
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
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Change Skybox", .font = m_font})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f }, .color = Silica::Color(150, 50, 50, 255),
						.onClick = [this]() {
							m_activeScene->removeSkybox();
							rebuildUI();
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Remove", .font = m_font})
					})}
				}
			});

			skyboxContent = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 4.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = skyPath.stem().string(), .font = m_font})},
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = skyRel.string(), .color = Silica::Color(150, 150, 150, 255), .font = m_font})},
					{ {0,0}, btnRow }
				}
			});
		}
		else {
			skyboxContent = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "No Skybox Loaded", .color = Silica::Color(150, 150, 150, 255), .font = m_font})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f }, .color = Silica::Color(50, 50, 50, 255),
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
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Select Skybox", .font = m_font})
					})}
				}
			});
		}
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Skybox", skyboxContent) });


		// -- Global Gravity --
		Vec3 currentGravity = m_activeScene->getGravity();
		auto gravityInput = Silica::MakeWidget<Silica::SVector3FloatInput>({
			.label = "",
			.initialValue = Silica::Vec3(currentGravity.x, currentGravity.y, currentGravity.z),
			.font = m_font,
			.onValueChanged = [this](Silica::Vec3 val) {
				m_activeScene->setGravity(Vec3(val.x, val.y, val.z));
			}
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Global Gravity", gravityInput) });


		// -- Ambient Color --
		auto ambientColorInput = MakeColorField(m_activeScene->getAmbientColor(), [this](Silica::Color c) {
			m_activeScene->setAmbientColor(Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f));
		});
		contentBox->addSlot({ {0, 0}, MakePropertyRow("Ambient Color", ambientColorInput) });


		// -- Final Layout Assembly --
		auto paddedContent = Silica::MakeWidget<Silica::SBox>({
			.padding = { 10.0f, 10.0f },
			.child = contentBox
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SScrollBox>({
			.child = paddedContent
		}));
	}

	void SceneOverviewPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(SceneOverviewPanel::onSceneChanged));
	}

	bool SceneOverviewPanel::onSceneChanged(SceneChangedEvent& e) {
		setScene(SceneManager::getScene());
		return false;
	}

	void SceneOverviewPanel::setScene(const Ref<Scene>& scene) {
		m_activeScene = scene;
		rebuildUI();
	}

}
