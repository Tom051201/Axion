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

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/scene/SceneManager.h"

#include "AxionStudio/Source/core/EditorEvents.h"
#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorSettings.h"
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
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({.hasBorder = true });
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void ViewportPanel::onEvent(Event& ev) {
		EventDispatcher dispatcher(ev);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(ViewportPanel::onProjectChanged));
		dispatcher.dispatch<EditorSettingsChangedEvent>(AX_BIND_EVENT_FN(ViewportPanel::onEditorSettingsChanged));
	}

	EventReply ViewportPanel::onProjectChanged(ProjectChangedEvent& ev) {
		rebuildUI();
		return EventReply::unhandled();
	}

	EventReply ViewportPanel::onEditorSettingsChanged(EditorSettingsChangedEvent& ev) {
		if (ev.hasChange(EditorSettingType::Viewport)) {
			rebuildUI();
		}
		return EventReply::unhandled();
	}

	void ViewportPanel::refresh() {
		rebuildUI();
	}

	void ViewportPanel::refreshToolbar() {
		EditorActionQueue::push(AX_BIND_FN(ViewportPanel::rebuildToolbar));
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

		Silica::TextureID texID = 0;
		if (m_requestViewportTexture) {
			texID = m_requestViewportTexture();
		}

		m_viewportImage = Silica::MakeWidget<Silica::SImage>({
			.textureID = texID,
			.desiredSize = { DEFAULT_VP_WIDTH, DEFAULT_VP_HEIGHT }
		});

		// -- Build Overlay Children Dynamically --
		std::vector<Silica::WidgetPtr> overlayChildren;
		overlayChildren.push_back(m_viewportImage);

		if (EditorSettings::viewportPanelShowRendererStats) {
			overlayChildren.push_back(Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Left,
				.verticalAlign = Silica::VerticalAlign::Top,
				.child = Silica::MakeWidget<Silica::SBox>({
					.padding = { EditorTheme::PADDING_MEDIUM, EditorTheme::PADDING_MEDIUM },
					.backgroundColor = Silica::Color(0, 0, 0, 150),
					.child = m_statsText
				})
			}));
		}

		m_viewportContainer = Silica::MakeWidget<Silica::SBox>({
			.hasBorder = true,
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
				.children = overlayChildren
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
				.child = Silica::MakeWidget<Silica::SAlign>({
					.horizontalAlign = Silica::HorizontalAlign::Center,
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = text })
				})
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
							{ {0,0}, SilicaHelpers::MakeCheckboxMenuItem("Show Render Stats", EditorSettings::viewportPanelShowRendererStats, [this](bool val) {
								EditorSettings::viewportPanelShowRendererStats = val;
								rebuildUI();
							})},
							{ {0,0}, SilicaHelpers::MakeCheckboxMenuItem("Invert Camera X-Axis", EditorSettings::viewportPanelInvertCameraX, [this](bool val) {
								EditorSettings::viewportPanelInvertCameraX = val;
								rebuildUI();
							})},
							{ {0,0}, SilicaHelpers::MakeCheckboxMenuItem("Invert Camera Y-Axis", EditorSettings::viewportPanelInvertCameraY, [this](bool val) {
								EditorSettings::viewportPanelInvertCameraY = val;
								rebuildUI();
							})},
						}
					})
				})
			})
		});


		// -- Build Left Row --
		auto leftRow = Silica::MakeWidget<Silica::SHorizontalBox>({.spacing = EditorTheme::SPACING_SMALL });

		leftRow->addSlot({ {0,0}, optionsMenu });

		if (m_gizmo) {
			auto translateBtn = makeSquareTextButton("T", m_gizmo->getMode() == GizmoMode::Translate, [this]() { // TODO: maybe gray out or so when no entity selected
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
				.hasBorder = true,
				.backgroundColor = Silica::GetTheme().Background_Popup,
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = EditorTheme::SPACING_SMALL,
					.slots = {
						{ {0,0}, SilicaHelpers::MakePropertyRow("Speed (3D)", Silica::MakeWidget<Silica::SSliderFloat>({
							.initialValue = m_camera->m_translationSpeed3D,
							.minValue = 0.0f,
							.maxValue = 25.0f,
							.snapStep = 1.0f,
							.onValueChanged = [this](float v) { m_camera->m_translationSpeed3D = v; }
						})) },
						{ {0,0}, SilicaHelpers::MakePropertyRow("Rotate (3D)", Silica::MakeWidget<Silica::SSliderFloat>({
							.initialValue = m_camera->m_rotationSpeed3D * 1000.0f,
							.minValue = 0.0f,
							.maxValue = 10.0f,
							.snapStep = 1.0f,
							.onValueChanged = [this](float v) { m_camera->m_rotationSpeed3D = (v / 1000.0f); }
						})) },
						{ {0,0}, SilicaHelpers::MakePropertyRow("Speed (2D)", Silica::MakeWidget<Silica::SSliderFloat>({
							.initialValue = m_camera->m_keyboardSpeed2D,
							.minValue = 0.0f,
							.maxValue = 100.0f,
							.snapStep = 5.0f,
							.onValueChanged = [this](float v) { m_camera->m_keyboardSpeed2D = v; }
						})) },
						{ {0,0}, SilicaHelpers::MakePropertyRow("Drag (2D)", Silica::MakeWidget<Silica::SSliderFloat>({
							.initialValue = m_camera->m_dragSpeed2D * 100.0f,
							.minValue = 0.0f,
							.maxValue = 10.0f,
							.snapStep = 1.0f,
							.onValueChanged = [this](float v) { m_camera->m_dragSpeed2D = (v / 100.0f); }
						})) },
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

	void ViewportPanel::loadSettings(const YAML::Node& editorSettings) {
		if (auto vpSettings = editorSettings["Viewport"]) {
			if (vpSettings["ShowRendererStats"]) EditorSettings::viewportPanelShowRendererStats = vpSettings["ShowRendererStats"].as<bool>();
			if (vpSettings["InvertCamX"]) EditorSettings::viewportPanelInvertCameraX = vpSettings["InvertCamX"].as<bool>();
			if (vpSettings["InvertCamY"]) EditorSettings::viewportPanelInvertCameraY = vpSettings["InvertCamY"].as<bool>();
			if (vpSettings["CamProjectionType"]) {
				if (vpSettings["CamProjectionType"].as<std::string>() == "Orthographic") { m_camera->setProjectionType(Camera::ProjectionType::Orthographic); }
				else { m_camera->setProjectionType(Camera::ProjectionType::Perspective); }
			}
			if (vpSettings["TranslationSpeedCam3D"]) m_camera->m_translationSpeed3D = vpSettings["TranslationSpeedCam3D"].as<float>();
			if (vpSettings["RotationSpeedCam3D"]) m_camera->m_rotationSpeed3D = vpSettings["RotationSpeedCam3D"].as<float>();
			if (vpSettings["KeyboardSpeedCam2D"]) m_camera->m_keyboardSpeed2D = vpSettings["KeyboardSpeedCam2D"].as<float>();
			if (vpSettings["DragSpeedCam2D"]) m_camera->m_dragSpeed2D = vpSettings["DragSpeedCam2D"].as<float>();
			if (vpSettings["FOVCam3D"]) m_camera->m_fov = vpSettings["FOVCam3D"].as<float>();
			if (vpSettings["ZoomCam2D"]) m_camera->m_zoom2D = vpSettings["ZoomCam2D"].as<float>();
			if (vpSettings["GizmoMode"]) {
				if (vpSettings["GizmoMode"].as<std::string>() == "Rotate") { m_gizmo->setMode(GizmoMode::Rotate); }
				else if (vpSettings["GizmoMode"].as<std::string>() == "Scale") { m_gizmo->setMode(GizmoMode::Scale); }
				else { m_gizmo->setMode(GizmoMode::Translate); }
			}
			if (vpSettings["GizmoSpace"]) {
				if (vpSettings["GizmoSpace"].as<std::string>() == "Local") { m_gizmo->setSpace(GizmoSpace::Local); }
				else { m_gizmo->setSpace(GizmoSpace::Global); }
			}
			if (vpSettings["GizmoScale"]) EditorSettings::viewportPanelGizmoScale = vpSettings["GizmoScale"].as<float>();
		}
	}

	void ViewportPanel::saveSettings(YAML::Emitter& out) const {
		std::string gizmoMode = "Translate";
		if (m_gizmo->getMode() == GizmoMode::Rotate) gizmoMode = "Rotate";
		else if (m_gizmo->getMode() == GizmoMode::Scale) gizmoMode = "Scale";

		out << YAML::Key << "Viewport" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "ShowRendererStats" << YAML::Value << EditorSettings::viewportPanelShowRendererStats;
		out << YAML::Key << "InvertCamX" << YAML::Value << EditorSettings::viewportPanelInvertCameraX;
		out << YAML::Key << "InvertCamY" << YAML::Value << EditorSettings::viewportPanelInvertCameraY;
		out << YAML::Key << "CamProjectionType" << YAML::Value << ((m_camera->getProjectionType() == Camera::ProjectionType::Perspective) ? "Perspective" : "Orthographic");
		out << YAML::Key << "TranslationSpeedCam3D" << YAML::Value << m_camera->m_translationSpeed3D;
		out << YAML::Key << "RotationSpeedCam3D" << YAML::Value << m_camera->m_rotationSpeed3D;
		out << YAML::Key << "KeyboardSpeedCam2D" << YAML::Value << m_camera->m_keyboardSpeed2D;
		out << YAML::Key << "DragSpeedCam2D" << YAML::Value << m_camera->m_dragSpeed2D;
		out << YAML::Key << "FOVCam3D" << YAML::Value << m_camera->m_fov;
		out << YAML::Key << "ZoomCam2D" << YAML::Value << m_camera->m_zoom2D;
		out << YAML::Key << "GizmoMode" << YAML::Value << gizmoMode;
		out << YAML::Key << "GizmoSpace" << YAML::Value << ((m_gizmo->getSpace() == GizmoSpace::Local) ? "Local" : "Global");
		out << YAML::Key << "GizmoScale" << YAML::Value << EditorSettings::viewportPanelGizmoScale;
		out << YAML::EndMap;
	}

}
