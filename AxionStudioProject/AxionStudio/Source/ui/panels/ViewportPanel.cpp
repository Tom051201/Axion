#include "ViewportPanel.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SImage.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SSliderFloat.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"
#include "AxionStudio/Vendor/Silica/include/SOverlay.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"

namespace Axion {

	void ViewportPanel::setup(EditorState* currentState, EditorState* prePauseState, int* stepFrames, EditorCamera* camera) {
		m_currentState = currentState;
		m_prePauseState = prePauseState;
		m_stepFrames = stepFrames;
		m_camera = camera;
	}

	void ViewportPanel::setCallbacks(std::function<void()> onPlay, std::function<void()> onSimulate, std::function<void()> onStop) {
		m_onPlay = onPlay;
		m_onSimulate = onSimulate;
		m_onStop = onStop;
	}

	Silica::WidgetPtr ViewportPanel::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		m_toolbarContainer = Silica::MakeWidget<Silica::SBox>({
			.padding = { 2.0f, 4.0f },
			.backgroundColor = Silica::Color(25, 25, 25, 255)
		});

		rebuildToolbar();

		// -- Create Viewport Image And Stats Overlay --
		m_statsText = Silica::MakeWidget<Silica::STextBlock>({
			.text = "Stats",
			.color = Silica::Color(0, 255, 0, 255),
			.font = m_font
		});

		m_viewportImage = Silica::MakeWidget<Silica::SImage>({
			.textureID = 0,
			.tint = Silica::Color::white(),
			.desiredSize = { 1280.0f, 720.0f }
		});

		m_viewportContainer = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = Silica::Color(20, 20, 20, 255),
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

		// -- Helper Function --
		auto makeImageButton = [btnSize](Silica::TextureID texID, bool isDisabled, std::function<void()> onClick) {
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { 4.0f, 4.0f },
				.enabled = !isDisabled,
				.color = Silica::Color(45, 45, 45, 0),
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

		auto makeSliderRow = [this](const std::string& label, float& val, float min, float max) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{ 100,0 }, .child = Silica::MakeWidget<Silica::STextBlock>({.text = label, .font = m_font}) }) },
					{ {1,0}, Silica::MakeWidget<Silica::SSliderFloat>({.initialValue = val, .minValue = min, .maxValue = max, .onValueChanged = [&val](float v) { val = v; } }) }
				}
			});
		};


		// -- 2D / 3D Toggle Button --
		Silica::TextureID camTex = m_camera->is2D() ? SilicaContext::getIcon("2DCamIcon") : SilicaContext::getIcon("3DCamIcon");
		auto camBtn = makeImageButton(camTex, !isEdit, [this]() {
			if (m_camera->is2D()) m_camera->set3D(); else m_camera->set2D();
			refreshToolbar();
		});
		

		// -- Camera Settings Menu --
		auto camSettingsMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.anchorContent = Silica::MakeWidget<Silica::SBox>({
				.padding = { 4.0f, 4.0f },
				.backgroundColor = Silica::Color(45, 45, 45, 255),
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cam", .font = m_font})
			}),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 10.0f },
				.explicitSize = Silica::Vec2{ 250.0f, 0.0f },
				.backgroundColor = Silica::Color(35, 35, 35, 255),
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


		// -- Control Buttons --
		auto simBtn = makeImageButton(isSim ? SilicaContext::getIcon("StopButton") : SilicaContext::getIcon("SimulateButton"), isPlay, [this, isSim]() {
			if (isSim) m_onStop(); else m_onSimulate();
			refreshToolbar();
		});

		auto playBtn = makeImageButton(isPlay ? SilicaContext::getIcon("StopButton") : SilicaContext::getIcon("PlayButton"), isSim, [this, isPlay]() {
			if (isPlay) m_onStop(); else m_onPlay();
			refreshToolbar();
		});

		auto pauseBtn = makeImageButton(isPaused ? SilicaContext::getIcon("PlayButton") : SilicaContext::getIcon("PauseButton"), isEdit, [this, isPaused]() {
			if (isPaused) {
				*m_currentState = *m_prePauseState;
			}
			else {
				*m_prePauseState = *m_currentState;
				*m_currentState = EditorState::Pause;
			}
			refreshToolbar();
		});

		auto stepBtn = makeImageButton(SilicaContext::getIcon("StepButton"), !isPaused, [this]() {
			*m_stepFrames = 1;
			refreshToolbar();
		});


		// -- Assemble Toolbar --
		auto buttonRow = Silica::MakeWidget<Silica::SHorizontalBox>({
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

		m_toolbarContainer->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = buttonRow
		}));
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

}
