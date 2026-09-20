#include "studiopch.h"
#include "AssetManagerPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SCollapsingHeader.h>
#include <Silica/include/SMenuAnchor.h>
#include <Silica/include/SImage.h>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/graphics/Mesh.h"
#include "AxionEngine/Source/graphics/Shader.h"
#include "AxionEngine/Source/graphics/Material.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/SilicaContext.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"
#include "AxionStudio/Source/ui/EditorTheme.h"

// ----- HELPER FUNCTIONS AND CONSTANTS -----
namespace {

	template<typename T>
	Silica::WidgetPtr buildAssetInfoWidget(const char* name, Axion::AssetManagerPanel* panel, std::function<Silica::WidgetPtr(Axion::Ref<T>)> elementFunc) {
		const auto& map = Axion::AssetManager::getMap<T>();
		std::string label = std::string(name) + " (" + std::to_string(map.size()) + ")";

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = Axion::EditorTheme::SPACING_MEDIUM });

		if (map.empty()) {
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
				.text = "No " + std::string(name) + " loaded",
				.color = Silica::GetTheme().Text_Dim
			}) });
		}
		else {
			for (const auto& [handle, asset] : map) {
				auto assetContent = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = Axion::EditorTheme::SPACING_SMALL });

				std::string filePath = Axion::AssetManager::getRelativeToAssets(Axion::AssetManager::getAssetFilePath<T>(handle)).string();
				assetContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Asset File: " + filePath }) });

				if (asset) {
					assetContent->addSlot({ {0,0}, elementFunc(asset) });
				}
				else {
					assetContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
						.text = std::string(name) + " data not loaded",
						.color = Silica::GetTheme().Text_Warning
					}) });
				}

				// -- Unique ID to track expanded state --
				std::string assetId = std::string(name) + "_" + handle.uuid.toString();

				auto assetHeader = Silica::MakeWidget<Silica::SCollapsingHeader>({
					.title = std::string(name) + " [" + handle.uuid.toString() + "]",
					.initiallyOpen = panel->checkAndRegisterExpanded(assetId),
					.onToggleOpen = [panel, assetId](bool isOpen) { panel->setExpanded(assetId, isOpen); },
					.content = Silica::MakeWidget<Silica::SBox>({
						.padding = { Axion::EditorTheme::PADDING_MEDIUM, Axion::EditorTheme::PADDING_SMALL },
						.child = assetContent
					}),
				});

				contentBox->addSlot({ {0,0}, assetHeader });
			}
		}

		std::string catName = name;

		return Silica::MakeWidget<Silica::SCollapsingHeader>({
			.title = label,
			.initiallyOpen = panel->checkAndRegisterExpanded(catName),
			.onToggleOpen = [panel, catName](bool isOpen) { panel->setExpanded(catName, isOpen); },
			.content = Silica::MakeWidget<Silica::SBox>({
				.padding = { Axion::EditorTheme::PADDING_LARGE, Axion::EditorTheme::PADDING_SMALL },
				.child = contentBox
			}),
		});
	}

}

namespace Axion {

	Silica::WidgetPtr AssetManagerPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({.borderThickness = Silica::GetTheme().Border_Thickness });
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void AssetManagerPanel::onEvent(Event& ev) {
		EventDispatcher dispatcher(ev);
		dispatcher.dispatch<SceneChangedEvent>(AX_BIND_EVENT_FN(onSceneChanged));
	}

	EventReply AssetManagerPanel::onSceneChanged(SceneChangedEvent& ev) {
		refresh();
		return EventReply::unhandled();
	}

	void AssetManagerPanel::refresh() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void AssetManagerPanel::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		// -- Options Menu --
		auto optionsMenu = Silica::MakeWidget<Silica::SAlign>({
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = Silica::MakeWidget<Silica::SMenuAnchor>({
				.openToRight = true,
				.anchorContent = Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
					.color = Silica::Color::transparent(),
					.hoverColor = EditorTheme::BUTTON_COLOR_HOVER_SUBTLE,
					.onClick = []() { return Silica::EventReply::unhandled(); },
					.child = Silica::MakeWidget<Silica::SImage>({
						.textureID = SilicaContext::getIcon("GearIcon"),
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
							{ {0,0}, SilicaHelpers::MakeOptionsMenuItem("Expand All", AX_BIND_FN(AssetManagerPanel::expandAll)) },
							{ {0,0}, SilicaHelpers::MakeOptionsMenuItem("Collapse All", AX_BIND_FN(AssetManagerPanel::collapseAll)) },
							{ {0,0}, SilicaHelpers::MakeOptionsMenuItem("Reload", AX_BIND_FN(AssetManagerPanel::refresh)) }
						}
					})
				})
			})
		});

		// ----- Refresh Button -----
		auto refreshButton = SilicaHelpers::MakeToolbarBtn("Refresh Assets", Silica::GetTheme().Accent_Primary, AX_BIND_FN(AssetManagerPanel::refresh));

		// ----- Toolbar -----
		auto toolbar = Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::TOOLBAR_PADDING_X, 0.0f },
			.explicitSize = Silica::Vec2{ 0.0f, EditorTheme::TOOLBAR_HEIGHT },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::TOOLBAR_SPACING,
				.slots = {
					{ {0,0}, optionsMenu },
					{ {0,0}, refreshButton }
				}
			})
		});


		// ----- Scrollable Content -----
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_LARGE });

		// -- Mesh Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Mesh>("Mesh", this, [&](Ref<Mesh> mesh) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Vertices", std::to_string(mesh->getVertexBuffer()->getVertexCount())) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Indices", std::to_string(mesh->getIndexCount())) });
			return box;
		}) });

		// -- Texture2D Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Texture2D>("Texture2D", this, [&](Ref<Texture2D> tex) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Width", std::to_string(tex->getWidth()) + " px") });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Height", std::to_string(tex->getHeight()) + " px") });
			return box;
		}) });

		// -- TextureCube Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<TextureCube>("TextureCube", this, [&](Ref<TextureCube> cube) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Face Width", std::to_string(cube->getFaceWidth()) + " px") });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Face Height", std::to_string(cube->getFaceHeight()) + " px") });
			return box;
		}) });

		// -- Material Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Material>("Material", this, [&](Ref<Material> material) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Name", material->getName()) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Pipeline", material->getPipelineHandle().isValid() ? material->getPipelineHandle().uuid.toString() : "Internal Default") });
			return box;
		}) });

		// -- Skybox Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Skybox>("Skybox", this, [&](Ref<Skybox> skybox) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Texture UUID", skybox->getTextureHandle().uuid.toString()) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Pipeline UUID", skybox->getPipelineHandle().isValid() ? skybox->getPipelineHandle().uuid.toString() : "Internal Default") });
			return box;
		}) });

		// -- Shader Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Shader>("Shader", this, [&](Ref<Shader> shader) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Name", shader->getName()) });
			return box;
		}) });

		// -- Pipeline Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Pipeline>("Pipeline", this, [&](Ref<Pipeline> pipeline) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			const auto& spec = pipeline->getSpecification();
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Color Format", EnumUtils::toString(spec.colorFormat)) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Depth Test", spec.depthTest ? "Enabled" : "Disabled") });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Topology", EnumUtils::toString(spec.topology)) });
			return box;
		}) });

		// -- AudioClip Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<AudioClip>("AudioClip", this, [&](Ref<AudioClip> clip) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("File", clip->getPath().string()) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Load Mode", EnumUtils::toString(clip->getMode())) });
			return box;
		}) });

		// -- PhysicsMaterial Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<PhysicsMaterial>("PhysicsMaterial", this, [&](Ref<PhysicsMaterial> physMat) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Static Friction", std::to_string(physMat->staticFriction)) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Dynamic Friction", std::to_string(physMat->dynamicFriction)) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Restitution", std::to_string(physMat->restitution)) });
			return box;
		}) });

		// -- Prefab Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Prefab>("Prefab", this, [&](Ref<Prefab> prefab) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_SMALL });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Entity Nodes", std::to_string(prefab->getEntityNode().size())) });
			return box;
		}) });


		// ----- Assemble -----
		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = { EditorTheme::PADDING_LARGE, EditorTheme::PADDING_LARGE },
				.child = contentBox
			})
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = toolbar,
			.contentArea = scrollBox
		}));

		m_forceExpandAll = false;
	}

	bool AssetManagerPanel::isExpanded(const std::string& key) const {
		return m_expandedCategories.find(key) != m_expandedCategories.end();
	}

	void AssetManagerPanel::setExpanded(const std::string& key, bool expanded) {
		if (expanded) m_expandedCategories.insert(key);
		else m_expandedCategories.erase(key);
	}

	bool AssetManagerPanel::checkAndRegisterExpanded(const std::string& key) {
		if (m_forceExpandAll) m_expandedCategories.insert(key);
		return isExpanded(key);
	}

	void AssetManagerPanel::expandAll() {
		m_forceExpandAll = true;
		refresh();
	}
	void AssetManagerPanel::collapseAll() {
		m_expandedCategories.clear();
		refresh();
	}

}
