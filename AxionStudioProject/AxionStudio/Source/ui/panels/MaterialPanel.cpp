#include "studiopch.h"
#include "MaterialPanel.h"

#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalSplitBox.h>
#include <Silica/include/SVerticalSplitBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SButton.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/Theme.h>
#include <Silica/include/SImage.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SInputFieldFloat.h>
#include <Silica/include/SColorField.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/SWrappedTextBlock.h>
#include <Silica/include/SScissorBox.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SCheckbox.h>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/graphics/Renderer.h"
#include "AxionEngine/Source/graphics/Renderer3D.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionAssetPipeline/Source/parser/MaterialParser.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorSettings.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"
#include "AxionStudio/Source/ui/EditorTheme.h"

namespace {
	constexpr uint32_t PREVIEW_RES = 512;
	constexpr float MIN_SQUARE_RESERVE = 150.0f;
	constexpr float CAM_ZOOM_SPEED = 0.5f;
	constexpr float CAM_DRAG_SPEED = 0.01f;
	constexpr float CAM_PITCH_LIMIT = 1.5f;

	constexpr float TEX_SLOT_SIZE = 64.0f;
	constexpr float TEX_IMAGE_SIZE = 60.0f;


	// ----- HELPER WIDGETS -----
	class SPreviewLayout : public Silica::SWidget {
	public:

		struct Args {
			Silica::WidgetPtr leftPane;
			Silica::WidgetPtr rightPane;
		};

		void construct(const Args& args) {
			m_left = args.leftPane;
			m_right = args.rightPane;
		}

		void computeDesiredSize() override {
			m_desiredSize = { 0.0f, 0.0f };
			if (m_left) {
				m_left->computeDesiredSize();
				m_desiredSize.x += m_left->getDesiredSize().x;
				m_desiredSize.y = std::max(m_desiredSize.y, m_left->getDesiredSize().y);
			}
			if (m_right) {
				m_right->computeDesiredSize();
				m_desiredSize.x += m_right->getDesiredSize().x;
				m_desiredSize.y = std::max(m_desiredSize.y, m_right->getDesiredSize().y);
			}
		}

		void arrangeChildren(const Silica::Geometry& geom) override {
			SWidget::arrangeChildren(geom);
			float squareSize = std::min(geom.size.x, geom.size.y);
			if (geom.size.x - squareSize < MIN_SQUARE_RESERVE) squareSize = std::max(0.0f, geom.size.x - MIN_SQUARE_RESERVE);

			if (m_left) {
				Silica::Geometry leftGeom; leftGeom.position = geom.position; leftGeom.size = { squareSize, squareSize };
				m_left->arrangeChildren(leftGeom);
			}
			if (m_right) {
				Silica::Geometry rightGeom; rightGeom.position = { geom.position.x + squareSize, geom.position.y };
				rightGeom.size = { std::max(0.0f, geom.size.x - squareSize), geom.size.y };
				m_right->arrangeChildren(rightGeom);
			}
		}

		void onDraw(Silica::DrawList& dl, const Silica::Geometry& geom) const override {
			if (m_left) m_left->onDraw(dl, m_left->getAllocatedGeometry());
			if (m_right) m_right->onDraw(dl, m_right->getAllocatedGeometry());
		}

		void setRenderScale(float scale) override {
			m_renderScale = scale;
			if (m_left) m_left->setRenderScale(scale);
			if (m_right) m_right->setRenderScale(scale);
		}

		Silica::EventReply onMouseMove(const Silica::Geometry& geom, const Silica::Vec2& pos) override {
			Silica::EventReply r = Silica::EventReply::unhandled();
			if (m_right && m_right->getAllocatedGeometry().contains(pos)) r = m_right->onMouseMove(m_right->getAllocatedGeometry(), pos);
			if (!r.isHandled && m_left && m_left->getAllocatedGeometry().contains(pos)) r = m_left->onMouseMove(m_left->getAllocatedGeometry(), pos);
			return r;
		}

		Silica::EventReply onMouseButtonDown(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			Silica::EventReply r = Silica::EventReply::unhandled();
			if (m_right && m_right->getAllocatedGeometry().contains(pos)) r = m_right->onMouseButtonDown(m_right->getAllocatedGeometry(), pos, btn);
			if (!r.isHandled && m_left && m_left->getAllocatedGeometry().contains(pos)) r = m_left->onMouseButtonDown(m_left->getAllocatedGeometry(), pos, btn);
			return r;
		}

		Silica::EventReply onMouseButtonUp(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			Silica::EventReply r = Silica::EventReply::unhandled();
			if (m_right && m_right->getAllocatedGeometry().contains(pos)) r = m_right->onMouseButtonUp(m_right->getAllocatedGeometry(), pos, btn);
			if (!r.isHandled && m_left && m_left->getAllocatedGeometry().contains(pos)) r = m_left->onMouseButtonUp(m_left->getAllocatedGeometry(), pos, btn);
			return r;
		}

		Silica::EventReply onMouseWheel(const Silica::Geometry& geom, const Silica::Vec2& pos, float delta) override {
			Silica::EventReply r = Silica::EventReply::unhandled();
			if (m_right && m_right->getAllocatedGeometry().contains(pos)) r = m_right->onMouseWheel(m_right->getAllocatedGeometry(), pos, delta);
			if (!r.isHandled && m_left && m_left->getAllocatedGeometry().contains(pos)) r = m_left->onMouseWheel(m_left->getAllocatedGeometry(), pos, delta);
			return r;
		}

		Silica::EventReply onDragOver(const Silica::Geometry& geom, const Silica::Vec2& pos, const Silica::DragDropPayload& payload) override {
			Silica::EventReply r = Silica::EventReply::unhandled();
			if (m_right && m_right->getAllocatedGeometry().contains(pos)) r = m_right->onDragOver(m_right->getAllocatedGeometry(), pos, payload);
			if (!r.isHandled && m_left && m_left->getAllocatedGeometry().contains(pos)) r = m_left->onDragOver(m_left->getAllocatedGeometry(), pos, payload);
			return r;
		}

		Silica::EventReply onDrop(const Silica::Geometry& geom, const Silica::Vec2& pos, const Silica::DragDropPayload& payload) override {
			Silica::EventReply r = Silica::EventReply::unhandled();
			if (m_right && m_right->getAllocatedGeometry().contains(pos)) r = m_right->onDrop(m_right->getAllocatedGeometry(), pos, payload);
			if (!r.isHandled && m_left && m_left->getAllocatedGeometry().contains(pos)) r = m_left->onDrop(m_left->getAllocatedGeometry(), pos, payload);
			return r;
		}

	private:

		Silica::WidgetPtr m_left;
		Silica::WidgetPtr m_right;

	};

	class SCameraInteractBox : public Silica::SWidget {
	public:

		struct Args {
			float* pitch;
			float* yaw;
			float* distance;
			Silica::WidgetPtr child;
		};

		void construct(const Args& args) {
			m_pitch = args.pitch;
			m_yaw = args.yaw;
			m_distance = args.distance;
			m_child = args.child;
		}

		void computeDesiredSize() override {
			if (m_child) {
				m_child->computeDesiredSize();
				m_desiredSize = m_child->getDesiredSize();
			}
		}

		void arrangeChildren(const Silica::Geometry& geom) override {
			SWidget::arrangeChildren(geom);
			if (m_child) m_child->arrangeChildren(geom);
		}

		void onDraw(Silica::DrawList& dl, const Silica::Geometry& geom) const override {
			if (m_child) m_child->onDraw(dl, m_child->getAllocatedGeometry());
		}

		void setRenderScale(float scale) override {
			m_renderScale = scale;
			if (m_child) m_child->setRenderScale(scale);
		}

		Silica::EventReply onMouseButtonDown(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			if (geom.contains(pos) && btn == Silica::MouseButton::Right) {
				m_isDragging = true;
				m_lastMousePos = pos;
				Silica::SWidget::setCapturedWidget(this);
				return Silica::EventReply::handled();
			}
			if (m_child) return m_child->onMouseButtonDown(m_child->getAllocatedGeometry(), pos, btn);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseMove(const Silica::Geometry& geom, const Silica::Vec2& pos) override {
			if (m_isDragging) {
				float deltaX = pos.x - m_lastMousePos.x;
				float deltaY = pos.y - m_lastMousePos.y;
				m_lastMousePos = pos;
				float dragDirection = Axion::EditorSettings::materialEditorInvertCamera ? -1.0f : 1.0f;
				*m_yaw -= deltaX * CAM_DRAG_SPEED * dragDirection;
				*m_pitch -= deltaY * CAM_DRAG_SPEED * dragDirection;
				*m_pitch = std::clamp(*m_pitch, -CAM_PITCH_LIMIT, CAM_PITCH_LIMIT);

				return Silica::EventReply::handled();
			}
			if (m_child) return m_child->onMouseMove(m_child->getAllocatedGeometry(), pos);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonUp(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			if (m_isDragging && btn == Silica::MouseButton::Right) { m_isDragging = false; Silica::SWidget::setCapturedWidget(nullptr); return Silica::EventReply::handled(); }
			if (m_child) return m_child->onMouseButtonUp(m_child->getAllocatedGeometry(), pos, btn);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseWheel(const Silica::Geometry& geom, const Silica::Vec2& pos, float delta) override {
			if (geom.contains(pos)) {
				*m_distance -= delta * CAM_ZOOM_SPEED;
				*m_distance = std::clamp(*m_distance, 0.5f, 20.0f);
				return Silica::EventReply::handled();
			}
			if (m_child) return m_child->onMouseWheel(m_child->getAllocatedGeometry(), pos, delta);
			return Silica::EventReply::unhandled();
		}

	private:

		float* m_pitch;
		float* m_yaw;
		float* m_distance;
		bool m_isDragging = false;
		Silica::Vec2 m_lastMousePos = { 0, 0 };
		Silica::WidgetPtr m_child;

	};

}

namespace Axion {

	// ----- MATERIAL PANEL IMPLEMENTATION -----
	MaterialPanel::MaterialPanel() {
		FrameBufferSpecification spec;
		spec.width = PREVIEW_RES;
		spec.height = PREVIEW_RES;
		spec.textureFormat = ColorFormat::RGBA8;
		spec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		spec.clearColor = { 0.15f, 0.15f, 0.15f, 1.0f };
		m_previewFramebuffer = FrameBuffer::create(spec);
		m_viewportTextureID = SilicaContext::getFrameBufferTextureID(m_previewFramebuffer);

		m_previewCamera.setPerspective(Math::toRadians(45.0f), 0.1f, 100.0f);
		m_previewCamera.setViewportSize(PREVIEW_RES, PREVIEW_RES);
		m_previewCamera.setViewMatrix(Mat4::lookAt(Vec3(0.0f, 0.0f, 3.0f), Vec3::zero(), Vec3(0.0f, 1.0f, 0.0f)));
	}

	void MaterialPanel::setMaterial(const std::filesystem::path& materialPath) {
		m_currentMaterialPath = materialPath;
		UUID matUUID = AssetManager::getAssetUUID(materialPath);
		if (matUUID.isValid()) {
			m_materialHandle = AssetManager::load<Material>(matUUID);
			m_material = AssetManager::get<Material>(m_materialHandle);
		}
		else {
			m_material = nullptr;
		}

		EditorActionQueue::push([this]() { rebuildUI(); });
	}

	Silica::WidgetPtr MaterialPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.onDragOver = [](const Silica::DragDropPayload& payload) {
					if (payload.type == "AssetPath") {
						auto path = std::any_cast<std::filesystem::path>(payload.data);
						if (path.extension() == ".axmat") return Silica::EventReply::handled();
					}
					return Silica::EventReply::unhandled();
				},
				.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
					if (payload.type == "AssetPath") {
						auto path = std::any_cast<std::filesystem::path>(payload.data);
						if (path.extension() == ".axmat") {
							EditorActionQueue::push([this, path]() mutable {
								setMaterial(path);
							});
							return Silica::EventReply::handled();
						}
					}
					return Silica::EventReply::unhandled();
				}
			});
			rebuildUI();
		}
		return m_uiRoot;
	}

	void MaterialPanel::cmdSaveMaterial() {
		if (!m_material) return;

		AAP::MaterialAssetData data;
		data.uuid = m_materialHandle.uuid;
		data.name = m_material->getName();

		MaterialProperties prop;
		prop.albedoColor = m_material->getAlbedoColor();
		prop.metalness = m_material->getMetalness();
		prop.roughness = m_material->getRoughness();
		prop.emissionStrength = m_material->getEmission();
		prop.tiling = 1.0f;
		prop.useNormalMap = 0.0f;
		prop.useMetalnessMap = 0.0f;
		prop.useRoughnessMap = 0.0f;
		prop.useOcclusionMap = 0.0f;
		prop.useEmissiveMap = 0.0f;

		auto registry = ProjectManager::getProject()->getAssetRegistry();

		auto attachTex = [&](TextureSlot slot, float& useFlag) {
			AssetHandle<Texture2D> texHandle = m_material->getTexture(slot);
			if (texHandle.isValid()) {
				useFlag = 1.0f;
				data.textures[slot] = registry->get(texHandle.uuid).filePath;
			}
		};

		float dummyAlbedoUse = 1.0f;
		attachTex(TextureSlot::Albedo, dummyAlbedoUse);
		attachTex(TextureSlot::Normal, prop.useNormalMap);
		attachTex(TextureSlot::Metalness, prop.useMetalnessMap);
		attachTex(TextureSlot::Roughness, prop.useRoughnessMap);
		attachTex(TextureSlot::Occlusion, prop.useOcclusionMap);
		attachTex(TextureSlot::Emissive, prop.useEmissiveMap);

		data.properties = prop;

		AssetHandle<Pipeline> pipe = m_material->getPipelineHandle();
		if (pipe.isValid()) data.pipelineAsset = registry->get(pipe.uuid).filePath;

		AAP::MaterialParser::createTextFile(data, m_currentMaterialPath);
		AX_CORE_LOG_INFO("Successfully saved Material: {0}", m_currentMaterialPath.filename().string());
	}

	void MaterialPanel::rebuildUI() {
		if (!m_uiRoot) return;

		if (!ProjectManager::hasProject()) {
			m_uiRoot->setChild(SilicaHelpers::MakeEmptyState("No Project Loaded.\n\nPlease load or create a project from the top menu bar to view material properties."));
			return;
		}

		if (!m_material) {
			m_uiRoot->setChild(SilicaHelpers::MakeEmptyState("No Material Loaded.\n\nPlease drag and drop a .axmat file here to view and edit its properties."));
			return;
		}

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
						.desiredSize = { 16.0f, 16.0f }
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
							{ {0,0}, SilicaHelpers::MakeCheckboxMenuItem("Invert Camera", EditorSettings::materialEditorInvertCamera, [this](bool val) {
								EditorSettings::materialEditorInvertCamera = val;
								if (m_eventCallback) {
									EditorSettingsChangedEvent e(EditorSettingType::MaterialEditorCamera);
									m_eventCallback(e);
								}
								rebuildUI();
							})},
							{ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) },
							{ {0,0}, SilicaHelpers::MakeContextMenuItem("Locate in Browser", [this]() { PlatformUtils::showInFileExplorer(m_currentMaterialPath); }) },
							{ {0,0}, SilicaHelpers::MakeContextMenuItem("Close Material", [this]() { setMaterial(""); }) }
						}
					})
				})
			})
		});

		// -- Custom Mesh Drop Zone --
		auto customMeshBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.color = (m_previewShape == PreviewShape::Custom) ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
			.onClick = [this]() {
				auto meshDir = ProjectManager::getProject()->getAssetsPath() / "meshes";
				if (!std::filesystem::exists(meshDir)) meshDir = ProjectManager::getProject()->getAssetsPath();
				std::filesystem::path absPath = FileDialogs::openFile({ {"Mesh Asset", "*.axmesh"} }, meshDir);

				if (absPath.extension() == ".axmesh") {
					EditorActionQueue::push([this, absPath]() {
						UUID meshUUID = AssetManager::getAssetUUID(absPath);
						if (meshUUID.isValid()) {
							m_customMeshHandle = AssetManager::load<Mesh>(meshUUID);
							m_customMeshName = absPath.stem().string();
							m_previewShape = PreviewShape::Custom;
							rebuildUI();
						}
					});
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center, .verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = m_previewShape == PreviewShape::Custom ? m_customMeshName : "Custom Mesh"})
			})
		});

		auto customMeshDropZone = Silica::MakeWidget<Silica::SAlign>({
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = SilicaHelpers::MakeAssetDropZone(".axmesh", [this](const std::filesystem::path& path) {
				EditorActionQueue::push([this, path]() {
					UUID meshUUID = AssetManager::getAssetUUID(path);
					if (meshUUID.isValid()) {
						m_customMeshHandle = AssetManager::load<Mesh>(meshUUID);
						m_customMeshName = path.stem().string();
						m_previewShape = PreviewShape::Custom;
						rebuildUI();
					}
				});
			}, customMeshBtn, { 0.0f, 0.0f })
		});

		// -- Fixed Height Toolbar --
		auto toolbar = Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::TOOLBAR_PADDING_X, 0.0f },
			.explicitSize = Silica::Vec2{ 0.0f, EditorTheme::TOOLBAR_HEIGHT },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::TOOLBAR_SPACING,
				.slots = {
					{ {0,0}, optionsMenu },
					{ {0,0}, SilicaHelpers::MakeToolbarBtn("Save Material", Silica::GetTheme().Accent_Success, [this]() {
						cmdSaveMaterial();
					}) },
					{ {0,0}, SilicaHelpers::MakeToolbarBtn("Sphere", (m_previewShape == PreviewShape::Sphere) ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(), [this]() {
						m_previewShape = PreviewShape::Sphere;
						rebuildUI();
					}) },
					{ {0,0}, SilicaHelpers::MakeToolbarBtn("Cube", (m_previewShape == PreviewShape::Cube) ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(), [this]() {
						m_previewShape = PreviewShape::Cube;
						rebuildUI();
					}) },
					{ {0,0}, customMeshDropZone }
				}
			})
		});

		// -- Layout --
		auto leftPane = Silica::MakeWidget<Silica::SBox>({.child = Silica::MakeWidget<SCameraInteractBox>({
			.pitch = &m_cameraPitch,
			.yaw = &m_cameraYaw,
			.distance = &m_cameraDistance,
			.child = Silica::MakeWidget<Silica::SImage>({
				.textureID = m_viewportTextureID,
				.desiredSize = { 10.0f, 10.0f }
			})
		}) });

		auto rightPane = Silica::MakeWidget<Silica::SScrollBox>({.child = buildProperties() });

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = toolbar,
			.contentArea = Silica::MakeWidget<SPreviewLayout>({.leftPane = leftPane, .rightPane = rightPane })
		}));
	}

	Silica::WidgetPtr MaterialPanel::buildProperties() {
		auto propsBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::PADDING_LARGE });

		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Material: " + m_material->getName() }) });

		auto ColorToVec4 = [](const Silica::Color& c) { return Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f); };
		auto Vec4ToColor = [](const Vec4& c) { return Silica::Color((uint8_t)(c.x * 255.f), (uint8_t)(c.y * 255.f), (uint8_t)(c.z * 255.f), (uint8_t)(c.w * 255.f)); };

		// -- Material Values --
		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });
		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Values", .color = Silica::GetTheme().Text_Dim }) });

		propsBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Albedo", Silica::MakeWidget<Silica::SColorField>({
			.initialColor = Vec4ToColor(m_material->getAlbedoColor()),
			.onColorChanged = [this, ColorToVec4](Silica::Color c) { m_material->setAlbedoColor(ColorToVec4(c)); }
		})) });
		propsBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Metalness", Silica::MakeWidget<Silica::SInputFieldFloat>({
			.initialValue = m_material->getMetalness(),
			.onValueChanged = [this](float v) { m_material->setMetalness(v); }
		})) });
		propsBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Roughness", Silica::MakeWidget<Silica::SInputFieldFloat>({
			.initialValue = m_material->getRoughness(),
			.onValueChanged = [this](float v) { m_material->setRoughness(v); }
		})) });
		propsBox->addSlot({ {0,0}, SilicaHelpers::MakePropertyRow("Emission", Silica::MakeWidget<Silica::SInputFieldFloat>({
			.initialValue = m_material->getEmission(),
			.onValueChanged = [this](float v) { m_material->setEmission(v); }
		})) });

		// -- Texture Slots --
		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });
		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Textures", .color = Silica::GetTheme().Text_Dim }) });

		propsBox->addSlot({ {0,0}, buildTextureSlot("Albedo", TextureSlot::Albedo) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Normal", TextureSlot::Normal) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Metalness", TextureSlot::Metalness) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Roughness", TextureSlot::Roughness) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Occlusion", TextureSlot::Occlusion) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Emissive", TextureSlot::Emissive) });

		return Silica::MakeWidget<Silica::SBox>({ .padding = { EditorTheme::PADDING_LARGE, EditorTheme::PADDING_LARGE }, .child = propsBox });
	}

	Silica::WidgetPtr MaterialPanel::buildTextureSlot(const std::string& label, TextureSlot slot) {
		AssetHandle<Texture2D> currentTexHandle = m_material->getTexture(slot);
		Silica::WidgetPtr slotContent = nullptr;

		if (currentTexHandle.isValid()) {
			Ref<Texture2D> tex = AssetManager::get<Texture2D>(currentTexHandle);
			slotContent = Silica::MakeWidget<Silica::SImage>({ .textureID = SilicaContext::getTextureID(tex), .desiredSize = { TEX_IMAGE_SIZE, TEX_IMAGE_SIZE } });
		}
		else {
			slotContent = Silica::MakeWidget<Silica::STextBlock>({ .text = "No\nTex" });
		}

		auto slotButton = Silica::MakeWidget<Silica::SButton>({
			.padding = { 0.0f, 0.0f }, .color = Silica::Color::transparent(),
			.onClick = [this, slot]() {
				auto texDir = ProjectManager::getProject()->getAssetsPath() / "textures";
				if (!std::filesystem::exists(texDir)) texDir = ProjectManager::getProject()->getAssetsPath();
				auto absPath = FileDialogs::openFile({ {"Texture Asset", "*.axtex"} }, texDir);

				if (absPath.extension() == ".axtex") {
					EditorActionQueue::push([this, absPath, slot]() {
						UUID texUUID = AssetManager::getAssetUUID(absPath);
						if (texUUID.isValid()) { m_material->setTexture(slot, AssetManager::load<Texture2D>(texUUID)); rebuildUI(); }
					});
				}
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::SAlign>({.horizontalAlign = Silica::HorizontalAlign::Center, .verticalAlign = Silica::VerticalAlign::Center, .child = slotContent })
		});

		auto dropZone = Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{ TEX_SLOT_SIZE, TEX_SLOT_SIZE }, .borderThickness = Silica::GetTheme().Border_Thickness,
			.child = SilicaHelpers::MakeAssetDropZone(".axtex", [this, slot](const std::filesystem::path& path) {
				EditorActionQueue::push([this, path, slot]() {
					UUID texUUID = AssetManager::getAssetUUID(path);
					if (texUUID.isValid()) { m_material->setTexture(slot, AssetManager::load<Texture2D>(texUUID)); rebuildUI(); }
				});
			}, slotButton, {0.0f, 0.0f})
		});

		Silica::WidgetPtr clearButton = Silica::MakeWidget<Silica::SBox>({ .backgroundColor = Silica::Color::transparent() });
		if (currentTexHandle.isValid()) {
			clearButton = Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL }, .color = Silica::Color::transparent(), .hoverColor = Silica::GetTheme().Accent_Danger,
				.onClick = [this, slot]() { EditorActionQueue::push([this, slot]() { m_material->setTexture(slot, AssetHandle<Texture2D>()); rebuildUI(); }); return Silica::EventReply::handled(); },
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X"})
			});
		}

		return Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = EditorTheme::PADDING_LARGE,
			.slots = {
				{ {0,0}, dropZone },
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({.verticalAlign = Silica::VerticalAlign::Center, .child = Silica::MakeWidget<Silica::STextBlock>({.text = label }) })},
				{ {1,0}, Silica::MakeWidget<Silica::SAlign>({.horizontalAlign = Silica::HorizontalAlign::Right, .verticalAlign = Silica::VerticalAlign::Center, .child = clearButton })}
			}
		});
	}

	void MaterialPanel::onUpdate(Timestep ts) {
		if (!m_material) return;

		float cy = std::cos(m_cameraYaw); float sy = std::sin(m_cameraYaw);
		float cp = std::cos(m_cameraPitch); float sp = std::sin(m_cameraPitch);
		m_previewCamera.setViewMatrix(Mat4::lookAt({ sy * cp * m_cameraDistance, sp * m_cameraDistance, cy * cp * m_cameraDistance }, Vec3::zero(), Vec3(0.0f, 1.0f, 0.0f)));

		LightingData lightData; lightData.ambientColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		lightData.directionalLights.push_back({ Vec3(-0.5f, -0.5f, -0.8f).normalized(), Vec4(3.0f, 3.0f, 3.0f, 1.0f) });
		lightData.pointLights.push_back({ Vec3(0.0f, 0.5f, 2.0f), Vec4(10.0f, 10.0f, 10.0f, 1.0f), 15.0f, 1.0f });

		m_previewFramebuffer->bind();
		m_previewFramebuffer->clear();
		Renderer3D::beginScene(m_previewCamera, lightData);

		if (SceneManager::getScene() && SceneManager::getScene()->hasSkybox()) {
			Ref<Skybox> skybox = AssetManager::get<Skybox>(SceneManager::getScene()->getSkyboxHandle());
			if (skybox) skybox->onUpdate(ts);
		}

		Ref<Mesh> mesh = nullptr;
		if (m_previewShape == PreviewShape::Sphere) mesh = EngineAssets::getSphereMesh();
		else if (m_previewShape == PreviewShape::Custom && m_customMeshHandle.isValid()) mesh = AssetManager::get<Mesh>(m_customMeshHandle);
		if (!mesh) mesh = EngineAssets::getCubeMesh();

		if (mesh) {
			ObjectBuffer objData;
			objData.modelMatrix = Mat4::TRS(Vec3::zero(), Quat::fromEulerAngles(Vec3::zero()), Vec3::one()).transposed().toXM();
			objData.color = m_material->getAlbedoColor().toFloat4();

			std::vector<ObjectBuffer> instanceData = { objData };
			uint32_t submeshCount = std::max((uint32_t)1, (uint32_t)mesh->getSubmeshes().size());
			for (uint32_t i = 0; i < submeshCount; i++) Renderer3D::drawMeshInstanced(mesh, i, m_material, instanceData);
		}

		Renderer3D::endScene();
		m_previewFramebuffer->unbind();
	}

	void MaterialPanel::onEvent(Event& ev) {
		EventDispatcher dispatcher(ev);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(onProjectChanged));
		dispatcher.dispatch<EditorSettingsChangedEvent>(AX_BIND_EVENT_FN(onEditorSettingsChanged));
	}

	EventReply MaterialPanel::onProjectChanged(ProjectChangedEvent& ev) {
		rebuildUI();
		return EventReply::unhandled();
	}

	EventReply MaterialPanel::onEditorSettingsChanged(EditorSettingsChangedEvent& ev) {
		rebuildUI();
		return EventReply::unhandled();
	}

}
