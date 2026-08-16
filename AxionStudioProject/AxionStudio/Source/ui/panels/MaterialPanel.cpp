#include "axpch.h"
#include "MaterialPanel.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalSplitBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalSplitBox.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/Theme.h"
#include "AxionStudio/Vendor/Silica/include/SImage.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/SInputFieldFloat.h"
#include "AxionStudio/Vendor/Silica/include/SColorField.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SSeparator.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EngineAssets.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/render/Renderer3D.h"
#include "AxionEngine/Source/scene/SceneManager.h"
#include "AxionEngine/Source/scene/Skybox.h"

#include "AxionAssetPipeline/Source/parser/MaterialParser.h"

namespace Axion {

	// ----- HELPER WIDGET -----
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

			if (geom.size.x - squareSize < 150.0f) {
				squareSize = std::max(0.0f, geom.size.x - 150.0f);
			}

			if (m_left) {
				Silica::Geometry leftGeom;
				leftGeom.position = geom.position;
				leftGeom.size = { squareSize, squareSize };
				m_left->arrangeChildren(leftGeom);
			}

			if (m_right) {
				Silica::Geometry rightGeom;
				rightGeom.position = { geom.position.x + squareSize, geom.position.y };
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

				*m_yaw -= deltaX * 0.01f;
				*m_pitch -= deltaY * 0.01f;
				*m_pitch = std::clamp(*m_pitch, -1.5f, 1.5f);

				return Silica::EventReply::handled();
			}
			if (m_child) return m_child->onMouseMove(m_child->getAllocatedGeometry(), pos);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseButtonUp(const Silica::Geometry& geom, const Silica::Vec2& pos, Silica::MouseButton btn) override {
			if (m_isDragging && btn == Silica::MouseButton::Right) {
				m_isDragging = false;
				Silica::SWidget::setCapturedWidget(nullptr);
				return Silica::EventReply::handled();
			}
			if (m_child) return m_child->onMouseButtonUp(m_child->getAllocatedGeometry(), pos, btn);
			return Silica::EventReply::unhandled();
		}

		Silica::EventReply onMouseWheel(const Silica::Geometry& geom, const Silica::Vec2& pos, float delta) override {
			if (geom.contains(pos)) {
				*m_distance -= delta * 0.5f;
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



	// ----- MATERIAL PANEL IMPLEMENTATION -----
	MaterialPanel::MaterialPanel() {
		// -- Initialize Framebuffer --
		FrameBufferSpecification spec;
		spec.width = 512;
		spec.height = 512;
		spec.textureFormat = ColorFormat::RGBA8;
		spec.depthStencilFormat = DepthStencilFormat::DEPTH32F;
		spec.clearColor = { 0.15f, 0.15f, 0.15f, 1.0f };
		m_previewFramebuffer = FrameBuffer::create(spec);

		m_viewportTextureID = SilicaContext::getFrameBufferTextureID(m_previewFramebuffer);

		// -- Initialize Preview Camera --
		m_previewCamera.setPerspective(Math::toRadians(45.0f), 0.1f, 100.0f);
		m_previewCamera.setViewportSize(512, 512);
		m_previewCamera.setViewMatrix(Mat4::lookAt(Vec3(0.0f, 0.0f, 3.0f), Vec3::zero(), Vec3(0.0f, 1.0f, 0.0f)));
	}

	void MaterialPanel::setMaterial(const std::filesystem::path& materialPath) {
		m_currentMaterialPath = materialPath;

		// -- Load Material --
		UUID matUUID = AssetManager::getAssetUUID(materialPath);
		if (matUUID.isValid()) {
			m_materialHandle = AssetManager::load<Material>(matUUID);
			m_material = AssetManager::get<Material>(m_materialHandle);
		}
		else {
			m_material = nullptr;
		}

		EditorActionQueue::push([this]() {
			rebuildUI();
		});
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

	void MaterialPanel::rebuildUI() {
		if (!m_uiRoot) return;

		// -- No Material --
		if (!m_material) {
			m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "No Material Loaded.\nOpen a .axmat file from the Content Browser." })
			}));
			return;
		}

		// -- Custom Mesh Drop Zone --
		auto customMeshDropZone = Silica::MakeWidget<Silica::SBox>({
			.backgroundColor = (m_previewShape == PreviewShape::Custom) ? Silica::GetTheme().Accent_Primary : Silica::Color(240, 40, 40, 255),
				.onDragOver = [](const Silica::DragDropPayload& payload) {
					if (payload.type == "AssetPath") {
						auto path = std::any_cast<std::filesystem::path>(payload.data);
						if (path.extension() == ".axmesh") return Silica::EventReply::handled();
					}
					return Silica::EventReply::unhandled();
				},
				.onDrop = [this](const Silica::DragDropPayload& payload) {
					if (payload.type == "AssetPath") {
						std::filesystem::path path = std::any_cast<std::filesystem::path>(payload.data);
						std::string ext = path.extension().string();

						// -- Validate Format --
						if (ext == ".axmesh") {
							EditorActionQueue::push([this, path]() {
								UUID meshUUID = AssetManager::getAssetUUID(path);
								if (meshUUID.isValid()) {
									m_customMeshHandle = AssetManager::load<Mesh>(meshUUID);
									m_customMeshName = path.stem().string();
									m_previewShape = PreviewShape::Custom;
									rebuildUI();
								}
							});
							return Silica::EventReply::handled();
						}
					}
					return Silica::EventReply::unhandled();
				},
				.child = Silica::MakeWidget<Silica::SButton>({
					.onClick = [this]() {
						std::filesystem::path meshDir = ProjectManager::getProject()->getAssetsPath() / "meshes";
						if (!std::filesystem::exists(meshDir)) {
							meshDir = ProjectManager::getProject()->getAssetsPath();
						}
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
						.horizontalAlign = Silica::HorizontalAlign::Center,
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = m_previewShape == PreviewShape::Custom ? m_customMeshName : "Custom Mesh"})
					})
				})
		});

		// -- Save Button --
		auto saveButton = Silica::MakeWidget<Silica::SButton>({
			.padding = {8, 4},
			.color = Silica::GetTheme().Accent_Success,
			.onClick = [this]() {
				if (!m_material) return Silica::EventReply::handled();

				// -- Initialize Asset Data --
				AAP::MaterialAssetData data;
				data.uuid = m_materialHandle.uuid;
				data.name = m_material->getName();

				// -- Fetch Active Properties --
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

				// -- Helper To Extract Texture Relative Paths From Active Asset Registry --
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

				// -- Fetch Active Pipeline path --
				AssetHandle<Pipeline> pipe = m_material->getPipelineHandle();
				if (pipe.isValid()) {
					data.pipelineAsset = registry->get(pipe.uuid).filePath;
				}

				// -- Write To Disk --
				AAP::MaterialParser::createTextFile(data, m_currentMaterialPath);

				AX_CORE_LOG_INFO("Successfully saved Material: {0}", m_currentMaterialPath.filename().string());

				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Save Material" })
			})
		});

		// -- Toolbar --
		auto toolbar = Silica::MakeWidget<Silica::SBox>({
			.padding = { 4.0f, 4.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 5.0f,
				.slots = {
					{ {0,0}, saveButton },
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = {8, 4},
						.color = (m_previewShape == PreviewShape::Sphere) ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
						.onClick = [this]() {
							m_previewShape = PreviewShape::Sphere;
							rebuildUI();
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Center,
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Sphere" })
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = {8, 4},
						.color = (m_previewShape == PreviewShape::Cube) ? Silica::GetTheme().Accent_Primary : Silica::Color::transparent(),
						.onClick = [this]() {
							m_previewShape = PreviewShape::Cube;
							rebuildUI();
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Center,
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Cube" })
						})
					})},
					{ {0,0}, customMeshDropZone }
				}
			})
		});

		// -- Left Pane Preview --
		auto viewportImage = Silica::MakeWidget<Silica::SImage>({
			.textureID = m_viewportTextureID,
			.desiredSize = { 10.0f, 10.0f }
		});

		auto cameraInteractBox = Silica::MakeWidget<SCameraInteractBox>({
			.pitch = &m_cameraPitch,
			.yaw = &m_cameraYaw,
			.distance = &m_cameraDistance,
			.child = viewportImage
		});

		auto leftPane = Silica::MakeWidget<Silica::SBox>({.child = cameraInteractBox });

		// -- Right Pane Properties --
		auto rightPane = Silica::MakeWidget<Silica::SScrollBox>({.child = buildProperties() });

		// -- Custom Layout --
		auto contentLayout = Silica::MakeWidget<SPreviewLayout>({
			.leftPane = leftPane,
			.rightPane = rightPane
		});

		// -- Final Assembly --
		auto mainLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = toolbar,
			.contentArea = contentLayout
		});

		m_uiRoot->setChild(mainLayout);
	}

	Silica::WidgetPtr MaterialPanel::buildProperties() {
		auto propsBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 15.0f });

		// -- Title --
		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = "Material: " + m_material->getName()
		}) });

		// -- Helper for Float Values --
		auto makeValueRow = [this](const std::string& label, float currentValue, std::function<void(float)> onCommit) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2{100.0f, 0.0f},
						.child = Silica::MakeWidget<Silica::SAlign>({
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = label})
						})
					})},
					{ {1,0}, Silica::MakeWidget<Silica::SInputFieldFloat>({
						.initialValue = currentValue,
						.onValueChanged = onCommit
					})}
				}
			});
		};

		// -- Helper for Color Values --
		auto makeColorRow = [this](const std::string& label, Vec4 currentColor, std::function<void(const Vec4&)> onCommit) {
			Silica::Color initialColor(
				(uint8_t)(currentColor.x * 255.0f),
				(uint8_t)(currentColor.y * 255.0f),
				(uint8_t)(currentColor.z * 255.0f),
				(uint8_t)(currentColor.w * 255.0f)
			);

			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2{100.0f, 0.0f},
						.child = Silica::MakeWidget<Silica::SAlign>({
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = label})
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SColorField>({
						.initialColor = initialColor,
						.onColorChanged = [onCommit](Silica::Color c) {
							onCommit(Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f));
						}
					})}
				}
			});
		};

		// -- Material Values --
		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({})});
		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = "Values",
			.color = Silica::GetTheme().Text_Dim
		}) });
		propsBox->addSlot({ {0,0}, makeColorRow("Albedo", m_material->getAlbedoColor(), [this](const Vec4& c) { m_material->setAlbedoColor(c); }) });
		propsBox->addSlot({ {0,0}, makeValueRow("Metalness", m_material->getMetalness(), [this](float v) { m_material->setMetalness(v); }) });
		propsBox->addSlot({ {0,0}, makeValueRow("Roughness", m_material->getRoughness(), [this](float v) { m_material->setRoughness(v); }) });
		propsBox->addSlot({ {0,0}, makeValueRow("Emission", m_material->getEmission(), [this](float v) { m_material->setEmission(v); }) });

		// -- Texture Slots --
		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });
		propsBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = "Textures",
			.color = Silica::GetTheme().Text_Dim
		}) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Albedo", TextureSlot::Albedo) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Normal", TextureSlot::Normal) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Metalness", TextureSlot::Metalness) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Roughness", TextureSlot::Roughness) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Occlusion", TextureSlot::Occlusion) });
		propsBox->addSlot({ {0,0}, buildTextureSlot("Emissive", TextureSlot::Emissive) });

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { 15.0f, 15.0f },
			.child = propsBox
		});
	}

	Silica::WidgetPtr MaterialPanel::buildTextureSlot(const std::string& label, TextureSlot slot) {

		AssetHandle<Texture2D> currentTexHandle = m_material->getTexture(slot);
		Silica::WidgetPtr slotContent = nullptr;
		if (currentTexHandle.isValid()) {
			Ref<Texture2D> tex = AssetManager::get<Texture2D>(currentTexHandle);
			Silica::TextureID texID = SilicaContext::getTextureID(tex);

			slotContent = Silica::MakeWidget<Silica::SImage>({
				.textureID = texID,
				.desiredSize = { 60.0f, 60.0f }
			});
		}
		else {
			slotContent = Silica::MakeWidget<Silica::STextBlock>({.text = "No\nTex" });
		}

		// -- Drop Zone Box --
		auto dropZone = Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{ 64.0f, 64.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.onDragOver = [](const Silica::DragDropPayload& payload) {
				if (payload.type == "AssetPath") {
					auto path = std::any_cast<std::filesystem::path>(payload.data);
					if (path.extension() == ".axtex") return Silica::EventReply::handled();
				}
				return Silica::EventReply::unhandled();
			},
			.onDrop = [this, slot](const Silica::DragDropPayload& payload) {
				if (payload.type == "AssetPath") {
					std::filesystem::path path = std::any_cast<std::filesystem::path>(payload.data);
					if (path.extension() == ".axtex") {
						EditorActionQueue::push([this, path, slot]() {
							UUID texUUID = AssetManager::getAssetUUID(path);
							if (texUUID.isValid()) {
								auto newTexHandle = AssetManager::load<Texture2D>(texUUID);
								m_material->setTexture(slot, newTexHandle);
								rebuildUI();
							}
						});
						return Silica::EventReply::handled();
					}
				}
				return Silica::EventReply::unhandled();
			},
			.child = Silica::MakeWidget<Silica::SButton>({
				.padding = { 0.0f, 0.0f },
				.color = Silica::Color::transparent(),
				.onClick = [this, slot]() {
					std::filesystem::path texDir = ProjectManager::getProject()->getAssetsPath() / "textures";
					if (!std::filesystem::exists(texDir)) {
						texDir = ProjectManager::getProject()->getAssetsPath();
					}
					std::filesystem::path absPath = FileDialogs::openFile({ {"Texture Asset", "*.axtex"} }, texDir);

					if (absPath.extension() == ".axtex") {
						EditorActionQueue::push([this, absPath, slot]() {
							UUID texUUID = AssetManager::getAssetUUID(absPath);
							if (texUUID.isValid()) {
								auto newTexHandle = AssetManager::load<Texture2D>(texUUID);
								m_material->setTexture(slot, newTexHandle);
								rebuildUI();
							}
						});
					}
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::SAlign>({
					.horizontalAlign = Silica::HorizontalAlign::Center,
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = slotContent
				})
			})
		});

		// -- Clear Button --
		Silica::WidgetPtr clearButton = nullptr;
		if (currentTexHandle.isValid()) {
			clearButton = Silica::MakeWidget<Silica::SButton>({
				.padding = {4, 4},
				.color = Silica::Color::transparent(),
				.hoverColor = Silica::GetTheme().Accent_Danger,
				.onClick = [this, slot]() {
					EditorActionQueue::push([this, slot]() {
						m_material->setTexture(slot, AssetHandle<Texture2D>());
						rebuildUI();
					});
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X"})
			});
		}
		else {
			clearButton = Silica::MakeWidget<Silica::SBox>({ .backgroundColor = Silica::Color::transparent() });
		}

		// -- Layout Label, Drop Zone, And Clear Button --
		return Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 15.0f,
			.slots = {
				{ {0,0}, dropZone },
				{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = label })
				})},
				{ {1,0}, Silica::MakeWidget<Silica::SAlign>({
					.horizontalAlign = Silica::HorizontalAlign::Right,
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = clearButton
				})}
			}
		});
	}

	void MaterialPanel::onUpdate(Timestep ts) {
		if (!m_material) return;

		// ----- Orbit Camera -----
		float cy = std::cos(m_cameraYaw);
		float sy = std::sin(m_cameraYaw);
		float cp = std::cos(m_cameraPitch);
		float sp = std::sin(m_cameraPitch);

		Vec3 camPos = {
			sy * cp * m_cameraDistance,
			sp * m_cameraDistance,
			cy * cp * m_cameraDistance
		};

		m_previewCamera.setViewMatrix(Mat4::lookAt(camPos, Vec3::zero(), Vec3(0.0f, 1.0f, 0.0f)));


		// ----- Studio Lighting Setup -----
		LightingData lightData;
		lightData.ambientColor = { 0.1f, 0.1f, 0.1f, 1.0f };

		Vec3 sunDir = Vec3(-0.5f, -0.5f, -0.8f).normalized();
		lightData.directionalLights.push_back({ sunDir, Vec4(3.0f, 3.0f, 3.0f, 1.0f) });

		lightData.pointLights.push_back({
			Vec3(0.0f, 0.5f, 2.0f),
			Vec4(10.0f, 10.0f, 10.0f, 1.0f),
			15.0f,
			1.0f
		});


		// ----- Render Scene -----
		m_previewFramebuffer->bind();
		m_previewFramebuffer->clear();

		Renderer3D::beginScene(m_previewCamera, lightData);

		// -- Render Skybox --
		if (SceneManager::getScene() && SceneManager::getScene()->hasSkybox()) {
			AssetHandle<Skybox> skyboxHandle = SceneManager::getScene()->getSkyboxHandle();
			Ref<Skybox> skybox = AssetManager::get<Skybox>(skyboxHandle);
			if (skybox) {
				skybox->onUpdate(ts);
			}
		}

		// -- Mesh Selection --
		Ref<Mesh> mesh = nullptr;

		if (m_previewShape == PreviewShape::Sphere) {
			mesh = EngineAssets::getSphereMesh();
		}
		else if (m_previewShape == PreviewShape::Custom && m_customMeshHandle.isValid()) {
			mesh = AssetManager::get<Mesh>(m_customMeshHandle);
		}

		if (!mesh) {
			mesh = EngineAssets::getCubeMesh();
		}

		if (mesh) {
			Mat4 transform = Mat4::TRS(Vec3::zero(), Quat::fromEulerAngles(Vec3::zero()), Vec3::one());

			ObjectBuffer objData;
			objData.modelMatrix = transform.transposed().toXM();
			objData.color = m_material->getAlbedoColor().toFloat4();

			std::vector<ObjectBuffer> instanceData = { objData };

			uint32_t submeshCount = std::max((uint32_t)1, (uint32_t)mesh->getSubmeshes().size());
			for (uint32_t i = 0; i < submeshCount; i++) {
				Renderer3D::drawMeshInstanced(mesh, i, m_material, instanceData);
			}
		}

		Renderer3D::endScene();

		m_previewFramebuffer->unbind();
	}

}
