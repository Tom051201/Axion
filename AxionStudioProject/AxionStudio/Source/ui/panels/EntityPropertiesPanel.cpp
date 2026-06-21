#include "EntityPropertiesPanel.h"

#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/SMenuAnchor.h"
#include "AxionStudio/Vendor/Silica/include/SCollapsingHeader.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SVector3Input.h"
#include "AxionStudio/Vendor/Silica/include/SVector3FloatInput.h"
#include "AxionStudio/Vendor/Silica/include/SCheckBox.h"
#include "AxionStudio/Vendor/Silica/include/SFloatInput.h"
#include "AxionStudio/Vendor/Silica/include/SColorPicker.h"

#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/scripting/ScriptEngine.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	// ----- CUSTOM WIDGETS -----
	class SEntityHeaderLayout : public Silica::SWidget {
	public:

		struct Args {
			Silica::WidgetPtr textInput;
			Silica::WidgetPtr addComponentButton;
		};

		void construct(const Args& args) {
			m_textInput = args.textInput;
			m_addButton = args.addComponentButton;
		}

		void computeDesiredSize() override {
			m_textInput->computeDesiredSize();
			m_addButton->computeDesiredSize();
			m_desiredSize.x = m_textInput->getDesiredSize().x + m_addButton->getDesiredSize().x + 10.0f;
			m_desiredSize.y = std::max(m_textInput->getDesiredSize().y, m_addButton->getDesiredSize().y);
		}

		void arrangeChildren(const Silica::Geometry& allocatedGeometry) override {
			SWidget::arrangeChildren(allocatedGeometry);

			float btnWidth = m_addButton->getDesiredSize().x;
			float spacing = 10.0f;

			Silica::Geometry textGeo;
			textGeo.position = allocatedGeometry.position;
			textGeo.size.x = std::max(0.0f, allocatedGeometry.size.x - btnWidth - spacing);
			textGeo.size.y = allocatedGeometry.size.y;
			m_textInput->arrangeChildren(textGeo);

			Silica::Geometry btnGeo;
			btnGeo.position.x = allocatedGeometry.position.x + allocatedGeometry.size.x - btnWidth;
			btnGeo.position.y = allocatedGeometry.position.y;
			btnGeo.size.x = btnWidth;
			btnGeo.size.y = allocatedGeometry.size.y;
			m_addButton->arrangeChildren(btnGeo);
		}

		void onDraw(Silica::DrawList& outDrawList, const Silica::Geometry& allocatedGeometry) const override {
			m_textInput->onDraw(outDrawList, m_textInput->getAllocatedGeometry());
			m_addButton->onDraw(outDrawList, m_addButton->getAllocatedGeometry());
		}

		Silica::EventReply onMouseMove(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos) override {
			if (m_addButton->onMouseMove(m_addButton->getAllocatedGeometry(), mousePos).isHandled) return Silica::EventReply::handled();
			return m_textInput->onMouseMove(m_textInput->getAllocatedGeometry(), mousePos);
		}

		Silica::EventReply onMouseButtonDown(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos, Silica::MouseButton button) override {
			if (m_addButton->onMouseButtonDown(m_addButton->getAllocatedGeometry(), mousePos, button).isHandled) return Silica::EventReply::handled();
			return m_textInput->onMouseButtonDown(m_textInput->getAllocatedGeometry(), mousePos, button);
		}

		Silica::EventReply onMouseButtonUp(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos, Silica::MouseButton button) override {
			if (m_addButton->onMouseButtonUp(m_addButton->getAllocatedGeometry(), mousePos, button).isHandled) return Silica::EventReply::handled();
			return m_textInput->onMouseButtonUp(m_textInput->getAllocatedGeometry(), mousePos, button);
		}

	private:

		Silica::WidgetPtr m_textInput;
		Silica::WidgetPtr m_addButton;

	};



	// ----- HELPER FUNCTIONS -----
	std::string ToLower(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
		return str;
	}

	template<typename T, typename UIBuilderFunc>
	void drawComponentBlock(const std::string& title, Entity entity, std::shared_ptr<Silica::SVerticalBox> container, Silica::FontAtlas* font, std::function<void()> triggerRebuild, bool isRemovable, UIBuilderFunc buildContent) {
		if (!entity.hasComponent<T>()) return;

		Silica::WidgetPtr removeButton = nullptr;
		if (isRemovable) {
			removeButton = Silica::MakeWidget<Silica::SButton>({
				.padding = { 4.0f, 0.0f },
				.color = Silica::Color::transparent(),
				.hoverColor = Silica::Color(200, 50, 50, 255),
				.onClick = [entity, triggerRebuild]() mutable {
					entity.removeComponent<T>();
					if (triggerRebuild) triggerRebuild();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X", .font = font})
			});
		}

		container->addSlot({
			.padding = { 0.0f, 0.0f },
			.child = Silica::MakeWidget<Silica::SCollapsingHeader>({
				.title = title,
				.initiallyOpen = true,
				.font = font,
				.trailingWidget = removeButton,
				.content = buildContent()
			})
		});
	}

	Silica::WidgetPtr MakeAddComponentItem(const std::string& text, Silica::FontAtlas* font, std::function<Silica::EventReply()> onClick) {
		return Silica::MakeWidget<Silica::SButton>({
			.padding = { 0.0f, 0.0f },
			.color = Silica::Color(45, 45, 45, 255),
			.hoverColor = Silica::Color(70, 130, 200, 255),
			.pressedColor = Silica::Color(50, 100, 180, 255),
			.onClick = onClick,
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = { 12.0f, 4.0f },
				.backgroundColor = Silica::Color::transparent(),
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = text, .font = font })
			})
		});
	}




	// ----- ENTITY PROPERTIES PANEL IMPLEMENTATION -----
	Silica::WidgetPtr EntityPropertiesPanel::getWidget(Silica::FontAtlas* font) {
		m_font = font;
		if (!m_uiRoot) {
			m_contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 4.0f });
			m_uiRoot = Silica::MakeWidget<Silica::SScrollBox>({ .child = m_contentBox });
			rebuildUI();
		}
		return m_uiRoot;
	}

	void EntityPropertiesPanel::setEntity(Entity entity) {
		m_selectedEntity = entity;
		EditorActionQueue::push([this]() { rebuildUI(); });
	}

	void EntityPropertiesPanel::setHierarchyRefreshCallback(std::function<void()> callback) {
		m_onHierarchyNeedsRefresh = callback;
	}

	void EntityPropertiesPanel::rebuildUI() {
		if (!m_contentBox) return;
		m_contentBox->clearSlots();

		Entity entity = m_selectedEntity;
		Silica::FontAtlas* font = m_font;
		auto container = m_contentBox;

		// -- Trigger Rebuild Helper Function --
		auto triggerRebuild = [this]() {
			if (m_onHierarchyNeedsRefresh) m_onHierarchyNeedsRefresh();
			EditorActionQueue::push([this]() { rebuildUI(); });
		};

		// -- No Entity Selected --
		if (!entity) {
			container->addSlot({
				.padding = { 10.0f, 10.0f },
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "Select an entity to view properties.",
					.font = font
				})
			});
			return;
		}

		// -- Tag And UUID --
		std::string tag = "Entity";
		if (entity.hasComponent<TagComponent>()) {
			tag = entity.getComponent<TagComponent>().tag;
		}

		std::string uuidStr = "UUID: Unknown";
		if (entity.hasComponent<UUIDComponent>()) {
			uuidStr = "UUID: " + entity.getComponent<UUIDComponent>().id.toString();
		}

		// -- Editable Text Field --
		auto nameInput = Silica::MakeWidget<Silica::SEditableText>({
			.initialText = tag,
			.font = font,
			.onTextCommitted = [entity, triggerRebuild](const std::string& newText) mutable {
				if (entity.hasComponent<TagComponent>()) {
					entity.getComponent<TagComponent>().tag = newText;
				}
				triggerRebuild();
			}
		});

		struct CompDef {
			std::string name;
			std::string category;
			std::function<void()> addFunc;
		};

		std::vector<CompDef> availableComps;

		auto registerComp = [&](const std::string& name, const std::string& category, auto addAction) {
			availableComps.push_back({ name, category, [addAction, triggerRebuild]() mutable {
				addAction();
				triggerRebuild();
			} });
		};

		if (!entity.hasComponent<MeshComponent>()) registerComp("Mesh", "Rendering", [=]() mutable { entity.addComponent<MeshComponent>(); });
		if (!entity.hasComponent<SkeletalMeshComponent>()) registerComp("Skeletal Mesh", "Rendering", [=]() mutable { entity.addComponent<SkeletalMeshComponent>(); });
		if (!entity.hasComponent<MaterialComponent>()) registerComp("Material", "Rendering", [=]() mutable { entity.addComponent<MaterialComponent>(); });
		if (!entity.hasComponent<SpriteComponent>()) registerComp("Sprite", "Rendering", [=]() mutable { entity.addComponent<SpriteComponent>(); });

		if (!entity.hasComponent<DirectionalLightComponent>()) registerComp("Directional Light", "Lighting", [=]() mutable { entity.addComponent<DirectionalLightComponent>(); });
		if (!entity.hasComponent<PointLightComponent>()) registerComp("Point Light", "Lighting", [=]() mutable { entity.addComponent<PointLightComponent>(); });
		if (!entity.hasComponent<SpotLightComponent>()) registerComp("Spot Light", "Lighting", [=]() mutable { entity.addComponent<SpotLightComponent>(); });

		if (!entity.hasComponent<RigidBodyComponent>()) registerComp("Rigid Body", "Physics", [=]() mutable { entity.addComponent<RigidBodyComponent>(); });
		if (!entity.hasComponent<BoxColliderComponent>()) registerComp("Box Collider", "Physics", [=]() mutable { entity.addComponent<BoxColliderComponent>(); });
		if (!entity.hasComponent<SphereColliderComponent>()) registerComp("Sphere Collider", "Physics", [=]() mutable { entity.addComponent<SphereColliderComponent>(); });
		if (!entity.hasComponent<CapsuleColliderComponent>()) registerComp("Capsule Collider", "Physics", [=]() mutable { entity.addComponent<CapsuleColliderComponent>(); });
		if (!entity.hasComponent<GravitySourceComponent>()) registerComp("Gravity Source", "Physics", [=]() mutable { entity.addComponent<GravitySourceComponent>(); });

		if (!entity.hasComponent<CameraComponent>()) registerComp("Camera", "General", [=]() mutable { entity.addComponent<CameraComponent>(); });
		if (!entity.hasComponent<AudioComponent>()) registerComp("Audio", "General", [=]() mutable { entity.addComponent<AudioComponent>(); });
		if (!entity.hasComponent<ParticleSystemComponent>()) registerComp("Particle System", "General", [=]() mutable { entity.addComponent<ParticleSystemComponent>(); });
		if (!entity.hasComponent<AnimatorComponent>()) registerComp("Animator", "General", [=]() mutable { entity.addComponent<AnimatorComponent>(); });

		if (!entity.hasComponent<NativeScriptComponent>()) registerComp("Native Script", "Scripting", [=]() mutable { entity.addComponent<NativeScriptComponent>(); });
		if (!entity.hasComponent<ScriptComponent>()) registerComp("C# Script", "Scripting", [=]() mutable { entity.addComponent<ScriptComponent>(); });

		auto menuListContainer = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });

		auto populateMenuList = [menuListContainer, availableComps, font](const std::string& filter) {
			menuListContainer->clearSlots();
			std::string lowerFilter = ToLower(filter);

			// -- Group Matches By Category --
			std::map<std::string, std::vector<CompDef>> grouped;
			for (const auto& comp : availableComps) {
				if (lowerFilter.empty() || ToLower(comp.name).find(lowerFilter) != std::string::npos) {
					grouped[comp.category].push_back(comp);
				}
			}

			if (grouped.empty()) {
				menuListContainer->addSlot({ .padding = {10,10}, .child = Silica::MakeWidget<Silica::STextBlock>({.text = "No matches found...", .font = font}) });
				return;
			}

			if (lowerFilter.empty()) {
				for (const auto& [category, comps] : grouped) {

					auto subMenuBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });
					for (const auto& comp : comps) {
						subMenuBox->addSlot({
							.padding = {0,0},
							.child = MakeAddComponentItem(comp.name, font, [comp]() {
								comp.addFunc();
								return Silica::EventReply::handled();
							})
							});
					}

					menuListContainer->addSlot({
						.padding = {0,0},
						.child = Silica::MakeWidget<Silica::SMenuAnchor>({
							.openOnHover = true,
							.openToRight = true,
							.showArrow = true,
							.anchorContent = MakeAddComponentItem(category, font, []() { return Silica::EventReply::unhandled(); }),
							.menuContent = Silica::MakeWidget<Silica::SBox>({
								.backgroundColor = Silica::Color(45, 45, 45, 255),
								.child = subMenuBox
							})
						})
					});
				}
			}
			else {
				for (const auto& [category, comps] : grouped) {
					for (const auto& comp : comps) {
						menuListContainer->addSlot({
							.padding = {0,0},
							.child = MakeAddComponentItem(comp.name + " (" + category + ")", font, [comp]() {
								comp.addFunc();
								return Silica::EventReply::handled();
							})
						});
					}
				}
			}
		};

		populateMenuList("");

		// -- Search Bar --
		auto searchBar = Silica::MakeWidget<Silica::SEditableText>({
			.hintText = "Search components...",
			.font = font,
			.onTextChanged = populateMenuList
		});


		// -- Add Component Button --
		auto addComponentMenu = Silica::MakeWidget<Silica::SMenuAnchor>({
			.openOnHover = false,
			.anchorContent = Silica::MakeWidget<Silica::SButton>({
				.padding = { 8.0f, 4.0f },
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Add Component", .font = font})
			}),
			.menuContent = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(45, 45, 45, 255),
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 2.0f,
					.slots = {
						{ {4.0f, 4.0f}, searchBar },
						{ {0.0f, 0.0f}, Silica::MakeWidget<Silica::SScrollBox>({.child = menuListContainer })}
					}
				})
			})
		});

		// -- Assemble Top Section --
		container->addSlot({
			.padding = { 5.0f, 5.0f },
			.child = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 4.0f,
				.slots = {
					{ { 0.0f, 0.0f }, Silica::MakeWidget<SEntityHeaderLayout>({
						.textInput = nameInput,
						.addComponentButton = addComponentMenu
					})},
					{ { 0.0f, 0.0f }, Silica::MakeWidget<Silica::STextBlock>({
						.text = uuidStr,
						.color = Silica::Color(150, 150, 150, 255),
						.font = font
					})}
				}
			})
		});

		// -- Helper Function --
		auto MakePropertyRow = [&](const std::string& label, Silica::WidgetPtr valueWidget) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(120.0f, 0.0f),
						.backgroundColor = Silica::Color::transparent(),
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = label, .font = font})
					})},
					{ {0, 0}, valueWidget }
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
				.anchorContent = Silica::MakeWidget<Silica::SButton>({
					.padding = { 2.0f, 2.0f },
					.child = Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(100.0f, 20.0f),
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


		// -- TRANSFORM COMPONENT --
		drawComponentBlock<TransformComponent>("Transform", entity, container, font, triggerRebuild, false, [&]() {
			auto& transform = entity.getComponent<TransformComponent>();
			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 4.0f,
					.slots = {
						{ {0,0}, Silica::MakeWidget<Silica::SVector3FloatInput>({
							.label = "Position",
							.initialValue = Silica::Vec3(transform.position.x, transform.position.y, transform.position.z),
							.font = font,
							.onValueChanged = [entity](Silica::Vec3 val) mutable {
								entity.getComponent<TransformComponent>().position = Vec3(val.x, val.y, val.z);
							}
						})},
						{ {0,0}, Silica::MakeWidget<Silica::SVector3FloatInput>({
							.label = "Rotation",
							.initialValue = Silica::Vec3(transform.getEulerAngles().x, transform.getEulerAngles().y, transform.getEulerAngles().z),
							.font = font,
							.onValueChanged = [entity](Silica::Vec3 val) mutable {
								entity.getComponent<TransformComponent>().setEulerAngles(Vec3(val.x, val.y, val.z));
							}
						})},
						{ {0,0}, Silica::MakeWidget<Silica::SVector3FloatInput>({
							.label = "Scale",
							.initialValue = Silica::Vec3(transform.scale.x, transform.scale.y, transform.scale.z),
							.font = font,
							.onValueChanged = [entity](Silica::Vec3 val) mutable {
								entity.getComponent<TransformComponent>().scale = Vec3(val.x, val.y, val.z);
							}
						})}
					}
				})
			});
		});


		// -- MESH COMPONENT --
		drawComponentBlock<MeshComponent>("Mesh", entity, container, font, triggerRebuild, true, [&]() {
			auto& meshComponent = entity.getComponent<MeshComponent>();

			if (meshComponent.handle.isValid()) {
				Ref<Mesh> mesh = AssetManager::get<Mesh>(meshComponent.handle);

				std::string vertexCount = mesh ? std::to_string(mesh->getVertexBuffer()->getVertexCount()) : "Unknown";
				std::string indexCount = mesh ? std::to_string(mesh->getIndexCount()) : "Unknown";

				return Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 5.0f },
					.child = Silica::MakeWidget<Silica::SVerticalBox>({
						.spacing = 4.0f,
						.slots = {
							{ {0, 0}, Silica::MakeWidget<Silica::STextBlock>({.text = "UUID: " + meshComponent.handle.uuid.toString(), .font = font}) },
							{ {0, 0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Vertices: " + vertexCount, .font = font}) },
							{ {0, 0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Indices: " + indexCount, .font = font}) },

							{ {0, 6}, Silica::MakeWidget<Silica::SButton>({
								.padding = { 8.0f, 4.0f },
								.onClick = [entity, triggerRebuild]() mutable {
									entity.getComponent<MeshComponent>().handle.invalidate();
									triggerRebuild();
									return Silica::EventReply::handled();
								},
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Clear Mesh", .font = font })
							})}
						}
					})
				});
			}
			else {
				return Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 5.0f },
					.child = Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.onClick = [entity, triggerRebuild]() mutable {
							std::filesystem::path meshDir = ProjectManager::getProject()->getAssetsPath() / "meshes";
							std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Mesh Asset", "*.axmesh"} }, meshDir);

							if (!absPath.empty()) {
								UUID assetUUID = AssetManager::getAssetUUID(absPath);
								if (assetUUID.isValid()) {
									entity.getComponent<MeshComponent>().handle = AssetManager::load<Mesh>(assetUUID);
									triggerRebuild();
								}
							}
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Open Mesh...", .font = font })
					})
				});
			}
		});


		// -- SKELETAL MESH COMPONENT --
		drawComponentBlock<SkeletalMeshComponent>("Skeletal Mesh", entity, container, font, triggerRebuild, true, [&]() {
			auto& skelMeshComp = entity.getComponent<SkeletalMeshComponent>();

			if (skelMeshComp.handle.isValid()) {
				Ref<SkeletalMesh> actualMesh = AssetManager::get<SkeletalMesh>(skelMeshComp.handle);

				std::string uuidStr = skelMeshComp.handle.uuid.toString();
				std::string vertexCount = actualMesh ? std::to_string(actualMesh->getVertexCount()) : "Unknown";
				std::string indexCount = actualMesh ? std::to_string(actualMesh->getIndexCount()) : "Unknown";
				std::string boneCount = actualMesh ? std::to_string(actualMesh->getSkeleton().bones.size()) : "Unknown";

				return Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 5.0f },
					.child = Silica::MakeWidget<Silica::SVerticalBox>({
						.spacing = 8.0f,
						.slots = {
							{ {0,0}, MakePropertyRow("UUID", Silica::MakeWidget<Silica::STextBlock>({.text = uuidStr, .font = font }))},
							{ {0,0}, MakePropertyRow("Vertices", Silica::MakeWidget<Silica::STextBlock>({.text = vertexCount, .font = font }))},
							{ {0,0}, MakePropertyRow("Indices", Silica::MakeWidget<Silica::STextBlock>({.text = indexCount, .font = font }))},
							{ {0,0}, MakePropertyRow("Bones", Silica::MakeWidget<Silica::STextBlock>({.text = boneCount, .font = font }))},

							{ {0, 6}, Silica::MakeWidget<Silica::SButton>({
								.padding = { 8.0f, 4.0f },
								.onClick = [entity, triggerRebuild]() mutable {
									entity.getComponent<SkeletalMeshComponent>().handle.invalidate();
									triggerRebuild();
									return Silica::EventReply::handled();
								},
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Clear Skeletal Mesh", .font = font })
							})}
						}
					})
				});
			}
			else {
				return Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 5.0f },
					.child = Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.onClick = [entity, triggerRebuild]() mutable {
							std::filesystem::path meshDir = ProjectManager::getProject()->getAssetsPath() / "meshes";
							if (!std::filesystem::exists(meshDir)) {
								meshDir = ProjectManager::getProject()->getAssetsPath();
							}

							std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Skeletal Mesh", "*.axskelmesh"} }, meshDir);

							if (!absPath.empty()) {
								UUID assetUUID = AssetManager::getAssetUUID(absPath);
								if (assetUUID.isValid()) {
									entity.getComponent<SkeletalMeshComponent>().handle = AssetManager::load<SkeletalMesh>(assetUUID);
									triggerRebuild();
								}
							}
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Open Skeletal Mesh...", .font = font })
					})
				});
			}
		});


		// -- MATERIAL COMPONENT --
		drawComponentBlock<MaterialComponent>("Material", entity, container, font, triggerRebuild, true, [&]() {
			auto& materialComponent = entity.getComponent<MaterialComponent>();

			uint32_t submeshCount = 1;
			if (entity.hasComponent<MeshComponent>()) {
				auto& meshComp = entity.getComponent<MeshComponent>();
				if (meshComp.handle.isValid()) {
					Ref<Mesh> mesh = AssetManager::get<Mesh>(meshComp.handle);
					if (mesh) {
						submeshCount = std::max((uint32_t)1, (uint32_t)mesh->getSubmeshes().size());
					}
				}
			}
			else if (entity.hasComponent<SkeletalMeshComponent>()) {
				auto& skelComp = entity.getComponent<SkeletalMeshComponent>();
				if (skelComp.handle.isValid()) {
					Ref<SkeletalMesh> mesh = AssetManager::get<SkeletalMesh>(skelComp.handle);
					if (mesh) {
						submeshCount = std::max((uint32_t)1, (uint32_t)mesh->getSubmeshes().size());
					}
				}
			}

			if (materialComponent.materials.size() < submeshCount) {
				materialComponent.materials.resize(submeshCount);
			}

			auto materialList = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 15.0f });

			for (uint32_t i = 0; i < submeshCount; i++) {
				std::string label = "Material (" + std::to_string(i) + ")";

				if (materialComponent.materials[i].isValid()) {
					Ref<Material> material = AssetManager::get<Material>(materialComponent.materials[i]);
					Ref<Pipeline> pipeline = AssetManager::get<Pipeline>(material->getPipelineHandle());

					std::string pipelineName = pipeline ? pipeline->getSpecification().shader->getName() : "Internal Default Pipeline";

					float* colorData = material->getAlbedoColor().data();
					Vec4 albedoVec4(colorData[0], colorData[1], colorData[2], colorData[3]);

					auto slotBox = Silica::MakeWidget<Silica::SBox>({
						.padding = { 10.0f, 5.0f },
						.backgroundColor = Silica::Color(35, 35, 35, 255),
						.child = Silica::MakeWidget<Silica::SVerticalBox>({
							.spacing = 8.0f,
							.slots = {
								{ {0,0}, MakePropertyRow(label, Silica::MakeWidget<Silica::STextBlock>({.text = material->getName(), .font = font }))},
								{ {0,0}, MakePropertyRow("Pipeline", Silica::MakeWidget<Silica::STextBlock>({.text = pipelineName, .font = font }))},
								{ {0,0}, MakePropertyRow("Albedo Color", MakeColorField(albedoVec4, [material](Silica::Color c) mutable {
									material->setAlbedoColor(Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f));
								}))},

								{ {0, 6}, Silica::MakeWidget<Silica::SButton>({
									.padding = { 8.0f, 4.0f },
									.onClick = [entity, i, triggerRebuild]() mutable {
										entity.getComponent<MaterialComponent>().materials[i].invalidate();
										triggerRebuild();
										return Silica::EventReply::handled();
									},
									.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Clear Material", .font = font })
								})}
							}
						})
					});

					materialList->addSlot({ .padding = {0,0}, .child = slotBox });
				}
				else {
					std::string btnLabel = "Open " + label + "...";

					auto slotBox = Silica::MakeWidget<Silica::SBox>({
						.padding = { 10.0f, 5.0f },
						.child = Silica::MakeWidget<Silica::SButton>({
							.padding = { 8.0f, 4.0f },
							.onClick = [entity, i, triggerRebuild]() mutable {
								std::filesystem::path materialDir = ProjectManager::getProject()->getAssetsPath() / "materials";
								if (!std::filesystem::exists(materialDir)) materialDir = ProjectManager::getProject()->getAssetsPath();

								std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Material Asset", "*.axmat"} }, materialDir);
								if (!absPath.empty()) {
									UUID assetUUID = AssetManager::getAssetUUID(absPath);
									if (assetUUID.isValid()) {
										entity.getComponent<MaterialComponent>().materials[i] = AssetManager::load<Material>(assetUUID);
										triggerRebuild();
									}
								}
								return Silica::EventReply::handled();
							},
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = btnLabel, .font = font })
						})
					});

					materialList->addSlot({ .padding = {0,0}, .child = slotBox });
				}
			}

			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 0.0f, 5.0f },
				.child = materialList
			});
		});


		// -- SPRITE COMPONENT --
		drawComponentBlock<SpriteComponent>("Sprite", entity, container, font, triggerRebuild, true, [&]() {
			auto& spriteComponent = entity.getComponent<SpriteComponent>();
			std::vector<Silica::Slot> uiSlots;

			uiSlots.push_back({
				{0,0}, MakePropertyRow("Tint", MakeColorField(spriteComponent.tint, [entity](Silica::Color c) mutable {
					entity.getComponent<SpriteComponent>().tint = Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f);
				}))
			});

			if (spriteComponent.texture.isValid()) {
				std::string uuidStr = spriteComponent.texture.uuid.toString();
				uiSlots.push_back({ {0,0}, MakePropertyRow("Texture UUID", Silica::MakeWidget<Silica::STextBlock>({.text = uuidStr, .font = font })) });

				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						entity.getComponent<SpriteComponent>().texture.invalidate();
						triggerRebuild();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Clear Texture", .font = font })
				}) });
			}
			else {
				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						std::filesystem::path texDir = ProjectManager::getProject()->getAssetsPath() / "textures";
						if (!std::filesystem::exists(texDir)) texDir = ProjectManager::getProject()->getAssetsPath();

						std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, texDir);
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) {
								entity.getComponent<SpriteComponent>().texture = AssetManager::load<Texture2D>(assetUUID);
								triggerRebuild();
							}
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Open Texture2D...", .font = font })
				}) });
			}

			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 8.0f, .slots = uiSlots })
				});
			});


		// -- DIRECTIONAL LIGHT COMPONENT --
		drawComponentBlock<DirectionalLightComponent>("Directional Light", entity, container, font, triggerRebuild, true, [&]() {
			auto& dirLightComponent = entity.getComponent<DirectionalLightComponent>();
			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 8.0f,
					.slots = {
						{ {0,0}, MakePropertyRow("Color", MakeColorField(dirLightComponent.color, [entity](Silica::Color c) mutable {
							entity.getComponent<DirectionalLightComponent>().color = Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f);
						}))}
					}
				})
			});
		});


		// -- POINT LIGHT COMPONENT --
		drawComponentBlock<PointLightComponent>("Point Light", entity, container, font, triggerRebuild, true, [&]() {
			auto& pointLightComponent = entity.getComponent<PointLightComponent>();
			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 8.0f,
					.slots = {
						{ {0,0}, MakePropertyRow("Color", MakeColorField(pointLightComponent.color, [entity](Silica::Color c) mutable {
							entity.getComponent<PointLightComponent>().color = Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f);
						}))},
						{ {0,0}, MakePropertyRow("Intensity", Silica::MakeWidget<Silica::SFloatInput>({
							.initialValue = pointLightComponent.intensity,
							.font = font,
							.onValueChanged = [entity](float val) mutable { entity.getComponent<PointLightComponent>().intensity = val; }
						}))},
						{ {0,0}, MakePropertyRow("Radius", Silica::MakeWidget<Silica::SFloatInput>({
							.initialValue = pointLightComponent.radius,
							.font = font,
							.onValueChanged = [entity](float val) mutable { entity.getComponent<PointLightComponent>().radius = val; }
						}))},
						{ {0,0}, MakePropertyRow("Falloff", Silica::MakeWidget<Silica::SFloatInput>({
							.initialValue = pointLightComponent.falloff,
							.font = font,
							.onValueChanged = [entity](float val) mutable { entity.getComponent<PointLightComponent>().falloff = val; }
						}))}
					}
				})
			});
		});


		// -- SPOT LIGHT COMPONENT --
		drawComponentBlock<SpotLightComponent>("Spot Light", entity, container, font, triggerRebuild, true, [&]() {
			auto& spotLightComponent = entity.getComponent<SpotLightComponent>();
			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 8.0f,
					.slots = {
						{ {0,0}, MakePropertyRow("Color", MakeColorField(spotLightComponent.color, [entity](Silica::Color c) mutable {
							entity.getComponent<SpotLightComponent>().color = Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f);
						}))},
						{ {0,0}, MakePropertyRow("Intensity", Silica::MakeWidget<Silica::SFloatInput>({
							.initialValue = spotLightComponent.intensity,
							.font = font,
							.onValueChanged = [entity](float val) mutable { entity.getComponent<SpotLightComponent>().intensity = val; }
						}))},
						{ {0,0}, MakePropertyRow("Range", Silica::MakeWidget<Silica::SFloatInput>({
							.initialValue = spotLightComponent.range,
							.font = font,
							.onValueChanged = [entity](float val) mutable { entity.getComponent<SpotLightComponent>().range = val; }
						}))},
						{ {0,0}, MakePropertyRow("Inner Cone", Silica::MakeWidget<Silica::SFloatInput>({
							.initialValue = spotLightComponent.innerConeAngle,
							.font = font,
							.onValueChanged = [entity](float val) mutable { entity.getComponent<SpotLightComponent>().innerConeAngle = val; }
						}))},
						{ {0,0}, MakePropertyRow("Outer Cone", Silica::MakeWidget<Silica::SFloatInput>({
							.initialValue = spotLightComponent.outerConeAngle,
							.font = font,
							.onValueChanged = [entity](float val) mutable { entity.getComponent<SpotLightComponent>().outerConeAngle = val; }
						}))}
					}
				})
			});
		});


		// -- CAMERA COMPONENT --
		drawComponentBlock<CameraComponent>("Camera", entity, container, font, triggerRebuild, true, [&]() {
			auto& cameraComponent = entity.getComponent<CameraComponent>();
			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 8.0f,
					.slots = {
						{ {0,0}, MakePropertyRow("Primary", Silica::MakeWidget<Silica::SCheckBox>({
							.initialCheck = cameraComponent.isPrimary,
							.onCheckChanged = [entity](bool checked) mutable {
								entity.getComponent<CameraComponent>().isPrimary = checked;
							}
						}))},
						{ {0,0}, MakePropertyRow("Fixed Aspect Ratio", Silica::MakeWidget<Silica::SCheckBox>({
							.initialCheck = cameraComponent.fixedAspectRatio,
							.onCheckChanged = [entity](bool checked) mutable {
								entity.getComponent<CameraComponent>().fixedAspectRatio = checked;
							}
						}))}
					}
				})
			});
		});


		// -- AUDIO COMPONENT --
		drawComponentBlock<AudioComponent>("Audio", entity, container, font, triggerRebuild, true, [&]() {
			auto& audioComponent = entity.getComponent<AudioComponent>();

			if (audioComponent.audio != nullptr) {
				Ref<AudioClip> clip = AssetManager::get<AudioClip>(audioComponent.audio->getClipHandle());
				std::string name = std::filesystem::path(clip->getPath()).filename().string();
				std::string uuid = audioComponent.audio->getClipHandle().uuid.toString();
				std::string mode = EnumUtils::toString(clip->getMode());
				bool isPaused = audioComponent.audio->isPaused();

				return Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 5.0f },
					.child = Silica::MakeWidget<Silica::SVerticalBox>({
						.spacing = 8.0f,
						.slots = {
							{ {0, 0}, MakePropertyRow("Name", Silica::MakeWidget<Silica::STextBlock>({.text = name, .font = font }))},
							{ {0, 0}, MakePropertyRow("UUID", Silica::MakeWidget<Silica::STextBlock>({.text = uuid, .font = font }))},
							{ {0, 0}, MakePropertyRow("Mode", Silica::MakeWidget<Silica::STextBlock>({.text = mode, .font = font }))},
							{ {0, 0}, MakePropertyRow("Playback", Silica::MakeWidget<Silica::SHorizontalBox>({
								.spacing = 4.0f,
								.slots = {
									{ {0,0}, Silica::MakeWidget<Silica::SButton>({
										.padding = { 8.0f, 2.0f },
										.onClick = [entity, triggerRebuild]() mutable {
											entity.getComponent<AudioComponent>().audio->play();
											triggerRebuild();
											return Silica::EventReply::handled();
										},
										.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Play", .font = font })
									})},
									{ {0,0}, Silica::MakeWidget<Silica::SButton>({
										.padding = { 8.0f, 2.0f },
										.onClick = [entity, triggerRebuild]() mutable {
											entity.getComponent<AudioComponent>().audio->stop();
											triggerRebuild();
											return Silica::EventReply::handled();
										},
										.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Stop", .font = font })
									})},
									{ {0,0}, Silica::MakeWidget<Silica::SButton>({
										.padding = { 8.0f, 2.0f },
										.onClick = [entity, isPaused, triggerRebuild]() mutable {
											if (isPaused) entity.getComponent<AudioComponent>().audio->resume();
											else entity.getComponent<AudioComponent>().audio->pause();
											triggerRebuild();
											return Silica::EventReply::handled();
										},
										.child = Silica::MakeWidget<Silica::STextBlock>({.text = isPaused ? "Resume" : "Pause", .font = font })
									})}
								}
							}))},
							{ {0, 0}, MakePropertyRow("Volume", Silica::MakeWidget<Silica::SFloatInput>({
								.initialValue = audioComponent.audio->getVolume(),
								.font = font,
								.onValueChanged = [entity](float val) mutable {
									entity.getComponent<AudioComponent>().audio->setVolume(val);
								}
							}))},
							{ {0, 0}, MakePropertyRow("Pitch", Silica::MakeWidget<Silica::SFloatInput>({
								.initialValue = audioComponent.audio->getPitch(),
								.font = font,
								.onValueChanged = [entity](float val) mutable {
									entity.getComponent<AudioComponent>().audio->setPitch(val);
								}
							}))},
							{ {0, 0}, MakePropertyRow("Pan", Silica::MakeWidget<Silica::SFloatInput>({
								.initialValue = audioComponent.audio->getPan(),
								.font = font,
								.onValueChanged = [entity](float val) mutable {
									entity.getComponent<AudioComponent>().audio->setPan(val);
								}
							}))},
							{ {0, 0}, MakePropertyRow("Loop", Silica::MakeWidget<Silica::SCheckBox>({
								.initialCheck = audioComponent.audio->isLooping(),
								.onCheckChanged = [entity](bool checked) mutable {
									entity.getComponent<AudioComponent>().audio->loop(checked);
								}
							}))},
							{ {0, 0}, MakePropertyRow("Spatialize", Silica::MakeWidget<Silica::SCheckBox>({
								.initialCheck = audioComponent.audio->isSpatial(),
								.onCheckChanged = [entity](bool checked) mutable {
									if (checked) entity.getComponent<AudioComponent>().audio->enableSpatial();
									else entity.getComponent<AudioComponent>().audio->disableSpatial();
								}
							}))},
							{ {0, 0}, MakePropertyRow("Min Distance", Silica::MakeWidget<Silica::SFloatInput>({
								.initialValue = audioComponent.audio->getMinDistance(),
								.font = font,
								.onValueChanged = [entity](float val) mutable {
									entity.getComponent<AudioComponent>().audio->setMinDistance(val);
								}
							}))},
							{ {0, 0}, MakePropertyRow("Max Distance", Silica::MakeWidget<Silica::SFloatInput>({
								.initialValue = audioComponent.audio->getMaxDistance(),
								.font = font,
								.onValueChanged = [entity](float val) mutable {
									entity.getComponent<AudioComponent>().audio->setMaxDistance(val);
								}
							}))},
							{ {0, 0}, MakePropertyRow("Doppler", Silica::MakeWidget<Silica::SFloatInput>({
								.initialValue = audioComponent.audio->getDopplerFactor(),
								.font = font,
								.onValueChanged = [entity](float val) mutable {
									entity.getComponent<AudioComponent>().audio->setDopplerFactor(val);
								}
							}))},
							{ {0, 6}, Silica::MakeWidget<Silica::SButton>({
								.padding = { 8.0f, 4.0f },
								.onClick = [entity, triggerRebuild]() mutable {
									entity.getComponent<AudioComponent>().audio = nullptr;
									triggerRebuild();
									return Silica::EventReply::handled();
								},
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Clear Audio Clip", .font = font })
							})}
						}
					})
				});
			}
			else {
				return Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 5.0f },
					.child = Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.onClick = [entity, triggerRebuild]() mutable {
							std::filesystem::path audioDir = ProjectManager::getProject()->getAssetsPath() / "audio";
							std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Audio Asset", "*.axaudio"} }, audioDir);

							if (!absPath.empty()) {
								UUID assetUUID = AssetManager::getAssetUUID(absPath);
								if (assetUUID.isValid()) {
									AssetHandle<AudioClip> clipHandle = AssetManager::load<AudioClip>(assetUUID);
									entity.getComponent<AudioComponent>().audio = std::make_shared<AudioSource>(clipHandle);
									triggerRebuild();
								}
							}
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Open Audio Clip...", .font = font })
					})
				});
			}
		});


		// -- SCRIPT COMPONENT --
		drawComponentBlock<ScriptComponent>("C# Script", entity, container, font, triggerRebuild, true, [&]() {
			auto& scriptComponent = entity.getComponent<ScriptComponent>();
			std::vector<Silica::Slot> uiSlots;

			uiSlots.push_back({ {0,0}, MakePropertyRow("Class Name", Silica::MakeWidget<Silica::SEditableText>({
				.initialText = scriptComponent.className,
				.font = font,
				.onTextCommitted = [entity](const std::string& newText) mutable {
					entity.getComponent<ScriptComponent>().className = newText;
				}
			})) });

			Silica::Color stateColor = scriptComponent.isInstantiated ? Silica::Color(50, 255, 50, 255) : Silica::Color(255, 200, 50, 255);
			std::string stateText = scriptComponent.isInstantiated ? "Running" : "Waiting to start";

			uiSlots.push_back({ {0,0}, MakePropertyRow("State", Silica::MakeWidget<Silica::STextBlock>({
				.text = stateText,
				.color = stateColor,
				.font = font
			})) });

			const auto& fields = ScriptEngine::getScriptFields(scriptComponent.className);

			if (!fields.empty()) {
				uiSlots.push_back({ {0, 10}, Silica::MakeWidget<Silica::STextBlock>({
					.text = "Script Variables",
					.color = Silica::Color(150, 150, 150, 255),
					.font = font
				}) });

				if (scriptComponent.isInstantiated && scriptComponent.gcHandle) {
					for (const auto& field : fields) {
						if (field.type == ScriptFieldType::Float) {
							float val = ScriptEngine::getFieldValueFloat(scriptComponent.gcHandle, field.name);
							uiSlots.push_back({ {0,0}, MakePropertyRow(field.name, Silica::MakeWidget<Silica::SFloatInput>({
								.initialValue = val,
								.font = font,
								.onValueChanged = [entity, fieldName = field.name](float newVal) mutable {
									auto& sc = entity.getComponent<ScriptComponent>();
									if (sc.isInstantiated && sc.gcHandle) {
										ScriptEngine::setFieldValueFloat(sc.gcHandle, fieldName, newVal);
									}
								}
							})) });
						}
						else if (field.type == ScriptFieldType::Vector3) {
							Vec3 val = ScriptEngine::getFieldValueVector3(scriptComponent.gcHandle, field.name);
							uiSlots.push_back({ {0,0}, Silica::MakeWidget<Silica::SVector3FloatInput>({
								.label = field.name,
								.initialValue = Silica::Vec3(val.x, val.y, val.z),
								.font = font,
								.labelWidth = 120.0f,
								.onValueChanged = [entity, fieldName = field.name](Silica::Vec3 newVal) mutable {
									auto& sc = entity.getComponent<ScriptComponent>();
									if (sc.isInstantiated && sc.gcHandle) {
										ScriptEngine::setFieldValueVector3(sc.gcHandle, fieldName, Vec3(newVal.x, newVal.y, newVal.z));
									}
								}
							}) });
						}
					}
				}
				else {
					for (const auto& field : fields) {
						if (field.type == ScriptFieldType::Float) {
							uiSlots.push_back({ {0,0}, MakePropertyRow(field.name, Silica::MakeWidget<Silica::STextBlock>({
								.text = "0.00 (Edit Mode)",
								.color = Silica::Color(100, 100, 100, 255),
								.font = font
							})) });
						}
						else if (field.type == ScriptFieldType::Vector3) {
							uiSlots.push_back({ {0,0}, MakePropertyRow(field.name, Silica::MakeWidget<Silica::STextBlock>({
								.text = "[ 0.00, 0.00, 0.00 ] (Edit Mode)",
								.color = Silica::Color(100, 100, 100, 255),
								.font = font
							})) });
						}
					}
				}
			}

			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 8.0f,
					.slots = uiSlots
				})
			});
		});


		// -- NATIVE SCRIPT COMPONENT --
		drawComponentBlock<NativeScriptComponent>("Native Script", entity, container, font, triggerRebuild, true, [&]() {
			auto& scriptComponent = entity.getComponent<NativeScriptComponent>();
			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({
					.spacing = 8.0f,
					.slots = {
						{ {0,0}, MakePropertyRow("Class Name", Silica::MakeWidget<Silica::STextBlock>({
							.text = scriptComponent.scriptName,
							.font = font
						}))}
					}
				})
			});
		});


		// -- PARTICLE SYSTEM COMPONENT --
		drawComponentBlock<ParticleSystemComponent>("Particle System", entity, container, font, triggerRebuild, true, [&]() {
			auto& particleSystemComponent = entity.getComponent<ParticleSystemComponent>();
			std::vector<Silica::Slot> uiSlots;

			uiSlots.push_back({ {0,0}, Silica::MakeWidget<Silica::SVector3FloatInput>({
				.label = "Velocity Var",
				.initialValue = Silica::Vec3(particleSystemComponent.velocityVariation.x, particleSystemComponent.velocityVariation.y, particleSystemComponent.velocityVariation.z),
				.font = font,
				.labelWidth = 120.0f,
				.onValueChanged = [entity](Silica::Vec3 val) mutable {
					entity.getComponent<ParticleSystemComponent>().velocityVariation = Vec3(val.x, val.y, val.z);
				}
			}) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Start Color", MakeColorField(particleSystemComponent.colorBegin, [entity](Silica::Color c) mutable {
				entity.getComponent<ParticleSystemComponent>().colorBegin = Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f);
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("End Color", MakeColorField(particleSystemComponent.colorEnd, [entity](Silica::Color c) mutable {
				entity.getComponent<ParticleSystemComponent>().colorEnd = Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f);
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Start Size", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = particleSystemComponent.sizeBegin,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<ParticleSystemComponent>().sizeBegin = val; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("End Size", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = particleSystemComponent.sizeEnd,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<ParticleSystemComponent>().sizeEnd = val; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Lifetime", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = particleSystemComponent.lifeTime,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<ParticleSystemComponent>().lifeTime = val; }
			})) });

			if (particleSystemComponent.texture.isValid()) {
				std::string uuidStr = particleSystemComponent.texture.uuid.toString();
				uiSlots.push_back({ {0,0}, MakePropertyRow("Texture UUID", Silica::MakeWidget<Silica::STextBlock>({.text = uuidStr, .font = font })) });
				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						entity.getComponent<ParticleSystemComponent>().texture.invalidate();
						triggerRebuild();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Clear Texture", .font = font })
				}) });
			}
			else {
				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						std::filesystem::path texDir = ProjectManager::getProject()->getAssetsPath() / "textures";
						if (!std::filesystem::exists(texDir)) texDir = ProjectManager::getProject()->getAssetsPath();

						std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Texture Asset", "*.axtex"} }, texDir);
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) {
								entity.getComponent<ParticleSystemComponent>().texture = AssetManager::load<Texture2D>(assetUUID);
								triggerRebuild();
							}
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Open Texture2D...", .font = font })
				}) });
			}

			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 8.0f, .slots = uiSlots })
			});
		});


		// -- ANIMATOR COMPONENT --
		drawComponentBlock<AnimatorComponent>("Animator", entity, container, font, triggerRebuild, true, [&]() {
			auto& animatorComponent = entity.getComponent<AnimatorComponent>();

			if (animatorComponent.currentClip.isValid()) {
				Ref<AnimationClip> clip = AssetManager::get<AnimationClip>(animatorComponent.currentClip);
				std::vector<Silica::Slot> uiSlots;

				std::string uuidStr = animatorComponent.currentClip.uuid.toString();
				uiSlots.push_back({ {0,0}, MakePropertyRow("Clip UUID", Silica::MakeWidget<Silica::STextBlock>({.text = uuidStr, .font = font })) });

				if (clip) {
					char durationBuf[64];
					snprintf(durationBuf, sizeof(durationBuf), "%.2f seconds", clip->duration);
					std::string durationStr = durationBuf;
					std::string trackCount = std::to_string(clip->boneAnimations.size());
					uiSlots.push_back({ {0,0}, MakePropertyRow("Duration", Silica::MakeWidget<Silica::STextBlock>({.text = durationStr, .font = font })) });
					uiSlots.push_back({ {0,0}, MakePropertyRow("Bone Tracks", Silica::MakeWidget<Silica::STextBlock>({.text = trackCount, .font = font })) });
				}

				uiSlots.push_back({ {0,0}, MakePropertyRow("Playing", Silica::MakeWidget<Silica::SCheckBox>({
					.initialCheck = animatorComponent.isPlaying,
					.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<AnimatorComponent>().isPlaying = checked; }
				})) });

				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						entity.getComponent<AnimatorComponent>().currentClip.invalidate();
						triggerRebuild();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Clear Animation Clip", .font = font })
				}) });

				return Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 5.0f },
					.child = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 8.0f, .slots = uiSlots })
				});
			}
			else {
				return Silica::MakeWidget<Silica::SBox>({
					.padding = { 10.0f, 5.0f },
					.child = Silica::MakeWidget<Silica::SButton>({
						.padding = { 8.0f, 4.0f },
						.onClick = [entity, triggerRebuild]() mutable {
							std::filesystem::path animDir = ProjectManager::getProject()->getAssetsPath() / "animations";
							if (!std::filesystem::exists(animDir)) animDir = ProjectManager::getProject()->getAssetsPath();

							std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Animation", "*.axanim"} }, animDir);
							if (!absPath.empty()) {
								UUID assetUUID = AssetManager::getAssetUUID(absPath);
								if (assetUUID.isValid()) {
									entity.getComponent<AnimatorComponent>().currentClip = AssetManager::load<AnimationClip>(assetUUID);
									triggerRebuild();
								}
							}
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Open Animation Clip...", .font = font })
					})
				});
			}
		});


		// -- RIGID BODY COMPONENT --
		drawComponentBlock<RigidBodyComponent>("Rigid Body", entity, container, font, triggerRebuild, true, [&]() {
			auto& rigidBodyComponent = entity.getComponent<RigidBodyComponent>();
			std::vector<Silica::Slot> uiSlots;

			std::string currentTypeStr = (rigidBodyComponent.type == RigidBodyComponent::BodyType::Static) ? "Static" : "Dynamic";
			auto bodyTypeCombo = Silica::MakeWidget<Silica::SMenuAnchor>({
				.openOnHover = false,
				.anchorContent = Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = currentTypeStr, .font = font})
				}),
				.menuContent = Silica::MakeWidget<Silica::SBox>({
					.backgroundColor = Silica::Color(45, 45, 45, 255),
					.child = Silica::MakeWidget<Silica::SVerticalBox>({
						.spacing = 0.0f,
						.slots = {
							{ {0,0}, Silica::MakeWidget<Silica::SButton>({
								.padding = { 8.0f, 4.0f },
								.color = Silica::Color::transparent(),
								.hoverColor = Silica::Color(70, 130, 200, 255),
								.onClick = [entity, triggerRebuild]() mutable {
									entity.getComponent<RigidBodyComponent>().type = RigidBodyComponent::BodyType::Static;
									triggerRebuild();
									return Silica::EventReply::handled();
								},
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Static", .font = font})
							})},
							{ {0,0}, Silica::MakeWidget<Silica::SButton>({
								.padding = { 8.0f, 4.0f },
								.color = Silica::Color::transparent(),
								.hoverColor = Silica::Color(70, 130, 200, 255),
								.onClick = [entity, triggerRebuild]() mutable {
									entity.getComponent<RigidBodyComponent>().type = RigidBodyComponent::BodyType::Dynamic;
									triggerRebuild();
									return Silica::EventReply::handled();
								},
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Dynamic", .font = font})
							})}
						}
					})
				})
			});

			uiSlots.push_back({ {0,0}, MakePropertyRow("Body Type", bodyTypeCombo) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Kinematic", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = rigidBodyComponent.isKinematic,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<RigidBodyComponent>().isKinematic = checked; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Use Global Gravity", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = rigidBodyComponent.useGlobalGravity,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<RigidBodyComponent>().useGlobalGravity = checked; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Enable CCD", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = rigidBodyComponent.enableCCD,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<RigidBodyComponent>().enableCCD = checked; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Mass", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = rigidBodyComponent.mass,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<RigidBodyComponent>().mass = val; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Linear Damping", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = rigidBodyComponent.linearDamping,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<RigidBodyComponent>().linearDamping = val; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Angular Damping", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = rigidBodyComponent.angularDamping,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<RigidBodyComponent>().angularDamping = val; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Fixed Rotation X", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = rigidBodyComponent.fixedRotationX,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<RigidBodyComponent>().fixedRotationX = checked; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Fixed Rotation Y", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = rigidBodyComponent.fixedRotationY,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<RigidBodyComponent>().fixedRotationY = checked; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Fixed Rotation Z", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = rigidBodyComponent.fixedRotationZ,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<RigidBodyComponent>().fixedRotationZ = checked; }
			})) });

			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 8.0f, .slots = uiSlots })
			});
		});


		// -- BOX COLLIDER COMPONENT --
		drawComponentBlock<BoxColliderComponent>("Box Collider", entity, container, font, triggerRebuild, true, [&]() {
			auto& boxColliderComponent = entity.getComponent<BoxColliderComponent>();
			std::vector<Silica::Slot> uiSlots;

			uiSlots.push_back({ {0,0}, Silica::MakeWidget<Silica::SVector3FloatInput>({
				.label = "Half Extents",
				.initialValue = Silica::Vec3(boxColliderComponent.halfExtents.x, boxColliderComponent.halfExtents.y, boxColliderComponent.halfExtents.z),
				.font = font,
				.labelWidth = 120.0f,
				.onValueChanged = [entity](Silica::Vec3 val) mutable {
					entity.getComponent<BoxColliderComponent>().halfExtents = Vec3(val.x, val.y, val.z);
				}
			}) });

			uiSlots.push_back({ {0,0}, Silica::MakeWidget<Silica::SVector3FloatInput>({
				.label = "Offset",
				.initialValue = Silica::Vec3(boxColliderComponent.offset.x, boxColliderComponent.offset.y, boxColliderComponent.offset.z),
				.font = font,
				.labelWidth = 120.0f,
				.onValueChanged = [entity](Silica::Vec3 val) mutable {
					entity.getComponent<BoxColliderComponent>().offset = Vec3(val.x, val.y, val.z);
				}
			}) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Is Trigger", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = boxColliderComponent.isTrigger,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<BoxColliderComponent>().isTrigger = checked; }
			})) });

			if (boxColliderComponent.material.isValid()) {
				Ref<PhysicsMaterial> material = AssetManager::get<PhysicsMaterial>(boxColliderComponent.material);
				if (material) {
					char sfBuf[32]; snprintf(sfBuf, sizeof(sfBuf), "%.2f (Read-Only)", material->staticFriction);
					char dfBuf[32]; snprintf(dfBuf, sizeof(dfBuf), "%.2f (Read-Only)", material->dynamicFriction);
					char resBuf[32]; snprintf(resBuf, sizeof(resBuf), "%.2f (Read-Only)", material->restitution);

					uiSlots.push_back({ {0,0}, MakePropertyRow("Static Friction", Silica::MakeWidget<Silica::STextBlock>({.text = sfBuf, .color = Silica::Color(150, 150, 150, 255), .font = font })) });
					uiSlots.push_back({ {0,0}, MakePropertyRow("Dynamic Friction", Silica::MakeWidget<Silica::STextBlock>({.text = dfBuf, .color = Silica::Color(150, 150, 150, 255), .font = font })) });
					uiSlots.push_back({ {0,0}, MakePropertyRow("Restitution", Silica::MakeWidget<Silica::STextBlock>({.text = resBuf, .color = Silica::Color(150, 150, 150, 255), .font = font })) });
				}

				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						entity.getComponent<BoxColliderComponent>().material.invalidate();
						triggerRebuild();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Remove Material", .font = font })
				}) });
			}
			else {
				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "physics";
						if (!std::filesystem::exists(dir)) dir = ProjectManager::getProject()->getAssetsPath();

						std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Physics Material Asset", "*.axpmat"} }, dir);
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) {
								entity.getComponent<BoxColliderComponent>().material = AssetManager::load<PhysicsMaterial>(assetUUID);
								triggerRebuild();
							}
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Load Material...", .font = font })
				}) });
			}

			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 8.0f, .slots = uiSlots })
			});
		});


		// -- SPHERE COLLIDER COMPOONENT --
		drawComponentBlock<SphereColliderComponent>("Sphere Collider", entity, container, font, triggerRebuild, true, [&]() {
			auto& sphereColliderComponent = entity.getComponent<SphereColliderComponent>();
			std::vector<Silica::Slot> uiSlots;

			uiSlots.push_back({ {0,0}, MakePropertyRow("Radius", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = sphereColliderComponent.radius,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<SphereColliderComponent>().radius = val; }
			})) });

			uiSlots.push_back({ {0,0}, Silica::MakeWidget<Silica::SVector3FloatInput>({
				.label = "Offset",
				.initialValue = Silica::Vec3(sphereColliderComponent.offset.x, sphereColliderComponent.offset.y, sphereColliderComponent.offset.z),
				.font = font,
				.labelWidth = 120.0f,
				.onValueChanged = [entity](Silica::Vec3 val) mutable {
					entity.getComponent<SphereColliderComponent>().offset = Vec3(val.x, val.y, val.z);
				}
			}) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Is Trigger", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = sphereColliderComponent.isTrigger,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<SphereColliderComponent>().isTrigger = checked; }
			})) });

			if (sphereColliderComponent.material.isValid()) {
				Ref<PhysicsMaterial> material = AssetManager::get<PhysicsMaterial>(sphereColliderComponent.material);
				if (material) {
					char sfBuf[32]; snprintf(sfBuf, sizeof(sfBuf), "%.2f (Read-Only)", material->staticFriction);
					char dfBuf[32]; snprintf(dfBuf, sizeof(dfBuf), "%.2f (Read-Only)", material->dynamicFriction);
					char resBuf[32]; snprintf(resBuf, sizeof(resBuf), "%.2f (Read-Only)", material->restitution);

					uiSlots.push_back({ {0,0}, MakePropertyRow("Static Friction", Silica::MakeWidget<Silica::STextBlock>({.text = sfBuf, .color = Silica::Color(150, 150, 150, 255), .font = font })) });
					uiSlots.push_back({ {0,0}, MakePropertyRow("Dynamic Friction", Silica::MakeWidget<Silica::STextBlock>({.text = dfBuf, .color = Silica::Color(150, 150, 150, 255), .font = font })) });
					uiSlots.push_back({ {0,0}, MakePropertyRow("Restitution", Silica::MakeWidget<Silica::STextBlock>({.text = resBuf, .color = Silica::Color(150, 150, 150, 255), .font = font })) });
				}

				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						entity.getComponent<SphereColliderComponent>().material.invalidate();
						triggerRebuild();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Remove Material", .font = font })
				}) });
			}
			else {
				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "physics";
						if (!std::filesystem::exists(dir)) dir = ProjectManager::getProject()->getAssetsPath();

						std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Physics Material Asset", "*.axpmat"} }, dir);
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) {
								entity.getComponent<SphereColliderComponent>().material = AssetManager::load<PhysicsMaterial>(assetUUID);
								triggerRebuild();
							}
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Load Material...", .font = font })
				}) });
			}

			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 8.0f, .slots = uiSlots })
			});
		});


		// -- CAPSULE COLLIDER COMPONENT --
		drawComponentBlock<CapsuleColliderComponent>("Capsule Collider", entity, container, font, triggerRebuild, true, [&]() {
			auto& capsuleColliderComponent = entity.getComponent<CapsuleColliderComponent>();
			std::vector<Silica::Slot> uiSlots;

			uiSlots.push_back({ {0,0}, MakePropertyRow("Radius", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = capsuleColliderComponent.radius,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<CapsuleColliderComponent>().radius = val; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Half Height", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = capsuleColliderComponent.halfHeight,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<CapsuleColliderComponent>().halfHeight = val; }
			})) });

			uiSlots.push_back({ {0,0}, Silica::MakeWidget<Silica::SVector3FloatInput>({
				.label = "Offset",
				.initialValue = Silica::Vec3(capsuleColliderComponent.offset.x, capsuleColliderComponent.offset.y, capsuleColliderComponent.offset.z),
				.font = font,
				.labelWidth = 120.0f,
				.onValueChanged = [entity](Silica::Vec3 val) mutable {
					entity.getComponent<CapsuleColliderComponent>().offset = Vec3(val.x, val.y, val.z);
				}
			}) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Is Trigger", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = capsuleColliderComponent.isTrigger,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<CapsuleColliderComponent>().isTrigger = checked; }
			})) });

			if (capsuleColliderComponent.material.isValid()) {
				Ref<PhysicsMaterial> material = AssetManager::get<PhysicsMaterial>(capsuleColliderComponent.material);
				if (material) {
					char sfBuf[32]; snprintf(sfBuf, sizeof(sfBuf), "%.2f (Read-Only)", material->staticFriction);
					char dfBuf[32]; snprintf(dfBuf, sizeof(dfBuf), "%.2f (Read-Only)", material->dynamicFriction);
					char resBuf[32]; snprintf(resBuf, sizeof(resBuf), "%.2f (Read-Only)", material->restitution);

					uiSlots.push_back({ {0,0}, MakePropertyRow("Static Friction", Silica::MakeWidget<Silica::STextBlock>({.text = sfBuf, .color = Silica::Color(150, 150, 150, 255), .font = font })) });
					uiSlots.push_back({ {0,0}, MakePropertyRow("Dynamic Friction", Silica::MakeWidget<Silica::STextBlock>({.text = dfBuf, .color = Silica::Color(150, 150, 150, 255), .font = font })) });
					uiSlots.push_back({ {0,0}, MakePropertyRow("Restitution", Silica::MakeWidget<Silica::STextBlock>({.text = resBuf, .color = Silica::Color(150, 150, 150, 255), .font = font })) });
				}

				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						entity.getComponent<CapsuleColliderComponent>().material.invalidate();
						triggerRebuild();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Remove Material", .font = font })
				}) });
			}
			else {
				uiSlots.push_back({ {0, 6}, Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.onClick = [entity, triggerRebuild]() mutable {
						std::filesystem::path dir = ProjectManager::getProject()->getAssetsPath() / "physics";
						if (!std::filesystem::exists(dir)) dir = ProjectManager::getProject()->getAssetsPath();

						std::filesystem::path absPath = FileDialogs::openFile({ {"Axion Physics Material Asset", "*.axpmat"} }, dir);
						if (!absPath.empty()) {
							UUID assetUUID = AssetManager::getAssetUUID(absPath);
							if (assetUUID.isValid()) {
								entity.getComponent<CapsuleColliderComponent>().material = AssetManager::load<PhysicsMaterial>(assetUUID);
								triggerRebuild();
							}
						}
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Load Material...", .font = font })
				}) });
			}

			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 8.0f, .slots = uiSlots })
			});
		});


		// -- GRAVITY SOURCE COMPONENT --
		drawComponentBlock<GravitySourceComponent>("Gravity Source", entity, container, font, triggerRebuild, true, [&]() {
			auto& gravitySourceComponent = entity.getComponent<GravitySourceComponent>();
			std::vector<Silica::Slot> uiSlots;

			std::string currentTypeStr = (gravitySourceComponent.type == GravitySourceComponent::Type::Directional) ? "Directional" : "Point";
			auto typeCombo = Silica::MakeWidget<Silica::SMenuAnchor>({
				.openOnHover = false,
				.anchorContent = Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = currentTypeStr, .font = font})
				}),
				.menuContent = Silica::MakeWidget<Silica::SBox>({
					.backgroundColor = Silica::Color(45, 45, 45, 255),
					.child = Silica::MakeWidget<Silica::SVerticalBox>({
						.spacing = 0.0f,
						.slots = {
							{ {0,0}, Silica::MakeWidget<Silica::SButton>({
								.padding = { 8.0f, 4.0f },
								.color = Silica::Color::transparent(),
								.hoverColor = Silica::Color(70, 130, 200, 255),
								.onClick = [entity, triggerRebuild]() mutable {
									entity.getComponent<GravitySourceComponent>().type = GravitySourceComponent::Type::Directional;
									triggerRebuild();
									return Silica::EventReply::handled();
								},
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Directional", .font = font})
							})},
							{ {0,0}, Silica::MakeWidget<Silica::SButton>({
								.padding = { 8.0f, 4.0f },
								.color = Silica::Color::transparent(),
								.hoverColor = Silica::Color(70, 130, 200, 255),
								.onClick = [entity, triggerRebuild]() mutable {
									entity.getComponent<GravitySourceComponent>().type = GravitySourceComponent::Type::Point;
									triggerRebuild();
									return Silica::EventReply::handled();
								},
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Point", .font = font})
							})}
						}
					})
				})
			});

			uiSlots.push_back({ {0,0}, MakePropertyRow("Type", typeCombo) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Affect Kinematic", Silica::MakeWidget<Silica::SCheckBox>({
				.initialCheck = gravitySourceComponent.affectKinematic,
				.onCheckChanged = [entity](bool checked) mutable { entity.getComponent<GravitySourceComponent>().affectKinematic = checked; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Strength", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = gravitySourceComponent.strength,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<GravitySourceComponent>().strength = val; }
			})) });

			uiSlots.push_back({ {0,0}, MakePropertyRow("Radius", Silica::MakeWidget<Silica::SFloatInput>({
				.initialValue = gravitySourceComponent.radius,
				.font = font,
				.onValueChanged = [entity](float val) mutable { entity.getComponent<GravitySourceComponent>().radius = val; }
			})) });

			return Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 5.0f },
				.child = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 8.0f, .slots = uiSlots })
			});
		});

	}

}
