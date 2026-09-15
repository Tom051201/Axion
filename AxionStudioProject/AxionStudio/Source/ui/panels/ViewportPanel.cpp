#include "studiopch.h"
#include "ViewportPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/Renderer.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SWrappedTextBlock.h>
#include <Silica/include/SScissorBox.h>
#include <Silica/include/SImage.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SSliderFloat.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SOverlay.h>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

#include "AxionStudio/Source/core/EditorEvents.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"
#include "AxionStudio/Source/ui/EditorTheme.h"

namespace {
	constexpr float GIZMO_BTN_SIZE = 32.0f;
	constexpr float DEFAULT_VP_WIDTH = 1280.0f;
	constexpr float DEFAULT_VP_HEIGHT = 720.0f;
}

namespace Axion {

	void ViewportPanel::setup(EditorState* currentState, EditorState* prePauseState, int* stepFrames, EditorCamera* camera, TransformGizmo* gizmo) {
		m_currentState = currentState;
		m_prePauseState = prePauseState;
		m_stepFrames = stepFrames;
		m_camera = camera;
		m_gizmo = gizmo;
	}

	Silica::WidgetPtr ViewportPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({ .borderThickness = Silica::GetTheme().Border_Thickness });
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void ViewportPanel::onEvent(Event& ev) {
		EventDispatcher dispatcher(ev);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(ViewportPanel::onProjectChanged));
	}

	EventReply ViewportPanel::onProjectChanged(ProjectChangedEvent& ev) {
		rebuildUI();
		return EventReply::unhandled();
	}

	void ViewportPanel::refresh() {
		rebuildUI();
	}

	void ViewportPanel::refreshToolbar() {
		EditorActionQueue::push([this]() { rebuildToolbar(); });
	}

	void ViewportPanel::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void ViewportPanel::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		// -- No Project Loaded --
		if (!ProjectManager::hasProject()) {
			m_viewportImage = nullptr;
			m_viewportContainer = nullptr;
			m_toolbarContainer = nullptr;
			m_statsText = nullptr;

			m_uiRoot->setChild(SilicaHelpers::MakeEmptyState("No Project Loaded.\n\nPlease load or create a project from the top menu bar to access the 3D Viewport.", 350.0f));
			return;
		}

		// -- Normal Active Viewport Toolbar --
		m_toolbarContainer = Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::TOOLBAR_PADDING_X, 0.0f },
			.explicitSize = Silica::Vec2{ 0.0f, EditorTheme::TOOLBAR_HEIGHT },
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
			.desiredSize = { DEFAULT_VP_WIDTH, DEFAULT_VP_HEIGHT }
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

						EditorActionQueue::push([this, path]() {
							UUID skyboxUUID = AssetManager::getAssetUUID(path);
							if (skyboxUUID.isValid()) {
								AssetHandle<Skybox> handle = AssetManager::load<Skybox>(skyboxUUID);
								Shared<Scene> scene = SceneManager::getScene();
								scene->setSkybox(handle);

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
							.padding = { EditorTheme::PADDING_MEDIUM, EditorTheme::PADDING_MEDIUM },
							.backgroundColor = Silica::Color(0, 0, 0, 150),
							.child = m_statsText
						})
					})
				}
			})
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = m_toolbarContainer,
			.contentArea = m_viewportContainer
			}));
	}

	void ViewportPanel::rebuildToolbar() {
		if (!m_toolbarContainer) return;

		// -- State Evaluation --
		bool isEdit = *m_currentState == EditorState::Edit;
		bool isPlay = *m_currentState == EditorState::Play || (*m_currentState == EditorState::Pause && *m_prePauseState == EditorState::Play);
		bool isSim = *m_currentState == EditorState::Simulate || (*m_currentState == EditorState::Pause && *m_prePauseState == EditorState::Simulate);
		bool isPaused = *m_currentState == EditorState::Pause;

		// -- Helper Functions --
		auto makeImageButton = [](Silica::TextureID texID, bool isDisabled, std::function<void()> onClick) {
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
				.enabled = !isDisabled,
				.color = Silica::Color::transparent(),
				.hoverColor = Silica::Color(100, 100, 100, 150),
				.disabledColor = Silica::Color::transparent(),
				.onClick = [onClick]() { onClick(); return Silica::EventReply::handled(); },
				.child = Silica::MakeWidget<Silica::SImage>({
					.textureID = texID,
					.tint = isDisabled ? Silica::Color(100, 100, 100, 150) : Silica::Color::white(),
					.desiredSize = { EditorTheme::ICON_SIZE_MEDIUM, EditorTheme::ICON_SIZE_MEDIUM }
				})
			});
		};

		auto makeTextButton = [](const std::string& text, bool isActive, std::function<void()> onClick) {
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
				.color = isActive ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
				.hoverColor = isActive ? Silica::GetTheme().Accent_Primary : Silica::Color(100, 100, 100, 150),
				.onClick = [onClick]() { onClick(); return Silica::EventReply::handled(); },
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = text })
			});
		};

		auto makeSquareTextButton = [](const std::string& text, bool isActive, std::function<void()> onClick) {
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { 0.0f, 0.0f },
				.color = isActive ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
				.hoverColor = isActive ? Silica::GetTheme().Accent_Primary : Silica::Color(100, 100, 100, 150),
				.onClick = [onClick]() { onClick(); return Silica::EventReply::handled(); },
				.child = Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2{ GIZMO_BTN_SIZE, GIZMO_BTN_SIZE },
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Center,
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = text })
					})
				})
			});
		};

		// -- Gear Options Menu --
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
							{ {0,0}, SilicaHelpers::MakeOptionsMenuItem("Reset View", [this]() {
								// Placeholder for later implementation
							}) }
						}
					})
				})
			})
		});


		// -- Build Left Row (Gear + Gizmo) --
		auto leftRow = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::SPACING_SMALL
		});

		leftRow->addSlot({ {0,0}, optionsMenu });

		if (m_gizmo) {
			auto translateBtn = makeSquareTextButton("T", m_gizmo->getMode() == GizmoMode::Translate, [this]() {
				m_gizmo->setMode(GizmoMode::Translate); refreshToolbar();
			});
			auto rotateBtn = makeSquareTextButton("R", m_gizmo->getMode() == GizmoMode::Rotate, [this]() {
				m_gizmo->setMode(GizmoMode::Rotate); refreshToolbar();
			});
			auto scaleBtn = makeSquareTextButton("S", m_gizmo->getMode() == GizmoMode::Scale, [this]() {
				m_gizmo->setMode(GizmoMode::Scale); refreshToolbar();
			});

			bool isLocal = m_gizmo->getSpace() == GizmoSpace::Local;
			auto spaceBtn = makeTextButton(isLocal ? "Local" : "World", false, [this, isLocal]() {
				m_gizmo->setSpace(isLocal ? GizmoSpace::Global : GizmoSpace::Local); refreshToolbar();
			});

			leftRow->addSlot({ {0,0}, translateBtn });
			leftRow->addSlot({ {0,0}, rotateBtn });
			leftRow->addSlot({ {0,0}, scaleBtn });
			leftRow->addSlot({ {0,0}, spaceBtn });
		}

		// -- Play / Simulate / Camera Tools --
		Silica::TextureID camTex = m_camera->is2D() ? SilicaContext::getIcon("2DCamIcon") : SilicaContext::getIcon("3DCamIcon");
		auto camBtn = makeImageButton(camTex, !isEdit, [this]() {
			if (m_camera->is2D()) m_camera->set3D();
			else m_camera->set2D();
			refreshToolbar();
		});

		auto camSettingsMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.anchorContent = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
				.color = Silica::Color::transparent(),
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cam" })
			}),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
				.explicitSize = Silica::Vec2{ EditorTheme::OPTIONS_MENU_WIDTH, 0.0f },
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::GetTheme().Background_Popup,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = EditorTheme::SPACING_SMALL,
					.slots = {
						{ {0,0}, SilicaHelpers::MakePropertyRow("Speed (3D)", Silica::MakeWidget<Silica::SSliderFloat>({.initialValue = m_camera->m_translationSpeed3D, .minValue = 0.0f, .maxValue = 25.0f, .onValueChanged = [this](float v) { m_camera->m_translationSpeed3D = v; } }), EditorTheme::PROPERTY_ROW_LABEL_WIDTH) },
						{ {0,0}, SilicaHelpers::MakePropertyRow("Rotate (3D)", Silica::MakeWidget<Silica::SSliderFloat>({.initialValue = m_camera->m_rotationSpeed3D, .minValue = 0.0f, .maxValue = 0.01f, .onValueChanged = [this](float v) { m_camera->m_rotationSpeed3D = v; } }), EditorTheme::PROPERTY_ROW_LABEL_WIDTH) },
						{ {0,0}, SilicaHelpers::MakePropertyRow("Speed (2D)", Silica::MakeWidget<Silica::SSliderFloat>({.initialValue = m_camera->m_keyboardSpeed2D, .minValue = 0.0f, .maxValue = 25.0f, .onValueChanged = [this](float v) { m_camera->m_keyboardSpeed2D = v; } }), EditorTheme::PROPERTY_ROW_LABEL_WIDTH) },
						{ {0,0}, SilicaHelpers::MakePropertyRow("Drag (2D)", Silica::MakeWidget<Silica::SSliderFloat>({.initialValue = m_camera->m_dragSpeed2D, .minValue = 0.0f, .maxValue = 0.1f, .onValueChanged = [this](float v) { m_camera->m_dragSpeed2D = v; } }), EditorTheme::PROPERTY_ROW_LABEL_WIDTH) }
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
			.spacing = EditorTheme::TOOLBAR_SPACING,
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
		auto toolbarOverlay = Silica::MakeWidget<Silica::SOverlay>({ .children = {} });

		toolbarOverlay->addChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Left,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = leftRow
		}));

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
		if (!m_viewportContainer) return false;
		if (Silica::Renderer::getOverlayManager().blocksInputAt(mousePos)) return false;
		return m_viewportContainer->getAllocatedGeometry().contains(mousePos);
	}

	Silica::Vec2 ViewportPanel::getRelativeMousePos() const {
		Silica::Vec2 globalMouse = Silica::Renderer::getMousePosition();
		Silica::Vec2 viewPos = getViewportPosition();
		return { globalMouse.x - viewPos.x, globalMouse.y - viewPos.y };
	}

}
