#include "studiopch.h"
#include "ViewportPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SImage.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SSliderFloat.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SOverlay.h>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

#include "AxionStudio/Source/core/EditorEvents.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"

namespace Axion {

	void ViewportPanel::setup(EditorState* currentState, EditorState* prePauseState, int* stepFrames, EditorCamera* camera, TransformGizmo* gizmo) {
		m_currentState = currentState;
		m_prePauseState = prePauseState;
		m_stepFrames = stepFrames;
		m_camera = camera;
		m_gizmo = gizmo;
	}

	Silica::WidgetPtr ViewportPanel::getWidget() {
		m_toolbarContainer = Silica::MakeWidget<Silica::SBox>({
			.padding = { 2.0f, 4.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
		});

		rebuildToolbar();

		// -- Create Viewport Image And Stats Overlay --
		m_statsText = Silica::MakeWidget<Silica::STextBlock>({
			.text = "Stats",
			.color = Silica::GetTheme().Text_Success
		});

		m_viewportImage = Silica::MakeWidget<Silica::SImage>({
			.textureID = 0,
			.desiredSize = { 1280.0f, 720.0f }
		});

		m_viewportContainer = Silica::MakeWidget<Silica::SBox>({
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.onDragOver = [](const Silica::DragDropPayload& payload) {
				if (payload.type == "AssetPath") {
					auto path = std::any_cast<std::filesystem::path>(payload.data);
					if (path.extension() == ".axsky" || path.extension() == ".axscene" || path.extension() == ".axprefab" || path.extension() == ".axvs") {
						return Silica::EventReply::handled();
					}
				}
				return Silica::EventReply::unhandled();
			},
			.onDrop = [this](const Silica::DragDropPayload& payload) {
				if (payload.type == "AssetPath") {
					auto path = std::any_cast<std::filesystem::path>(payload.data);
					if (path.extension() == ".axsky") {
						if (!SceneManager::hasScene()) return Silica::EventReply::unhandled();

						// -- Load Skybox From DragDrop --
						EditorActionQueue::push([this, path]() {
							UUID skyboxUUID = AssetManager::getAssetUUID(path);
							if (skyboxUUID.isValid()) {
								Shared<Scene> scene = SceneManager::getScene();
								scene->setSkybox(skyboxUUID);

								SceneModifiedEvent ev(SceneModificationType::SkyboxChanged);
								m_eventCallback(ev);

								AX_CORE_LOG_INFO("Successfully applied Skybox: {0}", path.filename().string());
							}
							else {
								AX_CORE_LOG_WARN("Attempted to drop an invalid Skybox asset!");
							}
						});

						return Silica::EventReply::handled();
					}
					else if (path.extension() == ".axscene") {
						EditorActionQueue::push([this, path]() {
							SceneManager::loadScene(path);
							AX_CORE_LOG_INFO("Successfully loaded Scene from drop: {0}", path.filename().string());
						});
						return Silica::EventReply::handled();
					}
					else if (path.extension() == ".axprefab" && m_onPrefabDropped) {
						Silica::Vec2 globalMouse = Silica::Renderer::getMousePosition();
						Silica::Vec2 viewPos = getViewportPosition();
						Silica::Vec2 localMouse = { globalMouse.x - viewPos.x, globalMouse.y - viewPos.y };

						EditorActionQueue::push([this, path, localMouse]() {
							m_onPrefabDropped(path, localMouse);
						});
						return Silica::EventReply::handled();
					}
					else if (path.extension() == ".axvs" && m_onVisualScriptDropped) {
						EditorActionQueue::push([this, path]() {
							m_onVisualScriptDropped(path);
						});
						return Silica::EventReply::handled();
					}
				}
				return Silica::EventReply::unhandled();
			},
			.child = Silica::MakeWidget<Silica::SOverlay>({
				.children = {
					m_viewportImage,
					Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Left,
						.verticalAlign = Silica::VerticalAlign::Top,
						.child = Silica::MakeWidget<Silica::SBox>({
							.padding = { 8.0f, 8.0f },
							.backgroundColor = Silica::Color(0, 0, 0, 150),
							.child = m_statsText
						})
					})
				}
			})
		});

		return Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = m_toolbarContainer,
			.contentArea = m_viewportContainer
		});
	}

	void ViewportPanel::refreshToolbar() {
		EditorActionQueue::push([this]() { rebuildToolbar(); });
	}

	void ViewportPanel::rebuildToolbar() {
		if (!m_toolbarContainer) return;

		// -- State Evaluation --
		bool isEdit = *m_currentState == EditorState::Edit;
		bool isPlay = *m_currentState == EditorState::Play || (*m_currentState == EditorState::Pause && *m_prePauseState == EditorState::Play);
		bool isSim = *m_currentState == EditorState::Simulate || (*m_currentState == EditorState::Pause && *m_prePauseState == EditorState::Simulate);
		bool isPaused = *m_currentState == EditorState::Pause;

		float btnSize = 24.0f;

		// -- Helper Functions --
		auto makeImageButton = [btnSize](Silica::TextureID texID, bool isDisabled, std::function<void()> onClick) {
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { 4.0f, 4.0f },
				.enabled = !isDisabled,
				.color = Silica::Color::transparent(),
				.hoverColor = Silica::Color(100, 100, 100, 150),
				.disabledColor = Silica::Color::transparent(),
				.onClick = [onClick]() {
					onClick();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::SImage>({
					.textureID = texID,
					.tint = isDisabled ? Silica::Color(100, 100, 100, 150) : Silica::Color::white(),
					.desiredSize = { btnSize, btnSize }
				})
			});
		};

		auto makeTextButton = [](const std::string& text, bool isActive, std::function<void()> onClick) {
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { 8.0f, 4.0f },
				.color = isActive ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
				.hoverColor = isActive ? Silica::GetTheme().Accent_Primary : Silica::Color(100, 100, 100, 150),
				.onClick = [onClick]() {
					onClick();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = text })
			});
		};

		auto makeSliderRow = [this](const std::string& label, float& val, float min, float max) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{ 100,0 }, .child = Silica::MakeWidget<Silica::STextBlock>({.text = label }) }) },
					{ {1,0}, Silica::MakeWidget<Silica::SSliderFloat>({.initialValue = val, .minValue = min, .maxValue = max, .onValueChanged = [&val](float v) { val = v; } }) }
				}
			});
		};

		auto makeSquareTextButton = [](const std::string& text, bool isActive, std::function<void()> onClick) {
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { 0.0f, 0.0f },
				.color = isActive ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
				.hoverColor = isActive ? Silica::GetTheme().Accent_Primary : Silica::Color(100, 100, 100, 150),
				.onClick = [onClick]() {
					onClick();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2{ 32.0f, 32.0f },
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Center,
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = text })
					})
				})
			});
		};


		// -- Gizmo Tools --
		Silica::WidgetPtr gizmoRow;
		if (m_gizmo) {
			auto translateBtn = makeSquareTextButton("T", m_gizmo->getMode() == GizmoMode::Translate, [this]() {
				m_gizmo->setMode(GizmoMode::Translate);
				refreshToolbar();
			});
			auto rotateBtn = makeSquareTextButton("R", m_gizmo->getMode() == GizmoMode::Rotate, [this]() {
				m_gizmo->setMode(GizmoMode::Rotate);
				refreshToolbar();
			});
			auto scaleBtn = makeSquareTextButton("S", m_gizmo->getMode() == GizmoMode::Scale, [this]() {
				m_gizmo->setMode(GizmoMode::Scale);
				refreshToolbar();
			});

			bool isLocal = m_gizmo->getSpace() == GizmoSpace::Local;
			auto spaceBtn = makeTextButton(isLocal ? "Local" : "World", false, [this, isLocal]() {
				m_gizmo->setSpace(isLocal ? GizmoSpace::Global : GizmoSpace::Local);
				refreshToolbar();
			});

			gizmoRow = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 4.0f,
				.slots = {
					{ {0,0}, translateBtn },
					{ {0,0}, rotateBtn },
					{ {0,0}, scaleBtn },
					{ {0,0}, spaceBtn }
				}
			});
		}

		// -- Play / Simulate / Camera Tools --
		Silica::TextureID camTex = m_camera->is2D() ? SilicaContext::getIcon("2DCamIcon") : SilicaContext::getIcon("3DCamIcon");
		auto camBtn = makeImageButton(camTex, !isEdit, [this]() {
			if (m_camera->is2D()) { m_camera->set3D(); }
			else { m_camera->set2D(); }
			refreshToolbar();
		});

		auto camSettingsMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.anchorContent = Silica::MakeWidget<Silica::SButton>({
				.padding = { 4.0f, 4.0f },
				.color = Silica::Color::transparent(),
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cam" })
			}),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 10.0f },
				.explicitSize = Silica::Vec2{ 250.0f, 0.0f },
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::GetTheme().Background_Popup,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 8.0f,
					.slots = {
						{ {0,0}, makeSliderRow("Speed (3D)", m_camera->m_translationSpeed3D, 0.0f, 25.0f) },
						{ {0,0}, makeSliderRow("Rotate (3D)", m_camera->m_rotationSpeed3D, 0.0f, 0.01f) },
						{ {0,0}, makeSliderRow("Speed (2D)", m_camera->m_keyboardSpeed2D, 0.0f, 25.0f) },
						{ {0,0}, makeSliderRow("Drag (2D)", m_camera->m_dragSpeed2D, 0.0f, 0.1f) }
					}
				})
			})
		});

		auto simBtn = makeImageButton(isSim ? SilicaContext::getIcon("StopButton") : SilicaContext::getIcon("SimulateButton"), isPlay, [this, isSim]() {
			if (m_eventCallback) {
				EditorStateChangedEvent e(isSim ? EditorState::Edit : EditorState::Simulate);
				m_eventCallback(e);
			}
			refreshToolbar();
		});

		auto playBtn = makeImageButton(isPlay ? SilicaContext::getIcon("StopButton") : SilicaContext::getIcon("PlayButton"), isSim, [this, isPlay]() {
			if (m_eventCallback) {
				EditorStateChangedEvent e(isPlay ? EditorState::Edit : EditorState::Play);
				m_eventCallback(e);
			}
			refreshToolbar();
		});

		auto pauseBtn = makeImageButton(isPaused ? SilicaContext::getIcon("PlayButton") : SilicaContext::getIcon("PauseButton"), isEdit, [this, isPaused]() {
			if (m_eventCallback) {
				EditorStateChangedEvent e(isPaused ? *m_prePauseState : EditorState::Pause);
				m_eventCallback(e);
			}
			refreshToolbar();
		});

		auto stepBtn = makeImageButton(SilicaContext::getIcon("StepButton"), !isPaused, [this]() {
			*m_stepFrames = 1;
			refreshToolbar();
		});

		auto centerRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 8.0f,
			.slots = {
				{ {0, 0}, camBtn },
				{ {0, 0}, camSettingsMenu },
				{ {0, 0}, simBtn },
				{ {0, 0}, playBtn },
				{ {0, 0}, pauseBtn },
				{ {0, 0}, stepBtn }
			}
		});

		// -- Assemble Overlay --
		auto toolbarOverlay = Silica::MakeWidget<Silica::SOverlay>({.children = {} });

		if (gizmoRow) {
			toolbarOverlay->addChild(Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Left,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = gizmoRow
			}));
		}

		toolbarOverlay->addChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = centerRow
		}));

		m_toolbarContainer->setChild(toolbarOverlay);
	}

	void ViewportPanel::setViewportTexture(Silica::TextureID texID, Silica::Vec2 size) {
		if (m_viewportImage) {
			m_viewportImage->setTextureID(texID);
			m_viewportImage->setDesiredSize(size);
		}
	}

	void ViewportPanel::setStatsText(const std::string& text) {
		if (m_statsText) m_statsText->setText(text);
	}

	Silica::Vec2 ViewportPanel::getViewportSize() const {
		if (m_viewportContainer) return m_viewportContainer->getAllocatedGeometry().size;
		return { 0.0f, 0.0f };
	}


	Silica::Vec2 ViewportPanel::getViewportPosition() const {
		if (m_viewportContainer) return m_viewportContainer->getAllocatedGeometry().position;
		return { 0.0f, 0.0f };
	}

	bool ViewportPanel::isHovered(const Silica::Vec2& mousePos) const {
		if (m_viewportContainer) return m_viewportContainer->getAllocatedGeometry().contains(mousePos);
		return false;
	}

	Silica::Vec2 ViewportPanel::getRelativeMousePos() const {
		Silica::Vec2 globalMouse = Silica::Renderer::getMousePosition();
		Silica::Vec2 viewPos = getViewportPosition();
		return { globalMouse.x - viewPos.x, globalMouse.y - viewPos.y };
	}

}
