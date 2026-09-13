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
#include "AxionStudio/Source/ui/SilicaHelpers.h"

// ----- HELPER FUNCTIONS AND CONSTANTS -----
namespace {
	constexpr float TOOLBAR_PADDING = 5.0f;
	constexpr float BUTTON_PAD_X = 10.0f;
	constexpr float BUTTON_PAD_Y = 6.0f;

	constexpr float SCROLL_CONTENT_PADDING = 10.0f;
	constexpr float SECTION_SPACING = 10.0f;
	constexpr float ROW_SPACING = 2.0f;
	constexpr float LIST_SPACING = 6.0f;
	constexpr float ASSET_ITEM_SPACING = 4.0f;

	constexpr float INNER_HEADER_PAD_X = 10.0f;
	constexpr float INNER_HEADER_PAD_Y = 5.0f;
	constexpr float OUTER_HEADER_PAD_X = 15.0f;
	constexpr float OUTER_HEADER_PAD_Y = 5.0f;


	template<typename T>
	Silica::WidgetPtr buildAssetInfoWidget(const char* name, std::function<Silica::WidgetPtr(Axion::Ref<T>)> elementFunc) {
		const auto& map = Axion::AssetManager::getMap<T>();
		std::string label = std::string(name) + " (" + std::to_string(map.size()) + ")";

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = LIST_SPACING });

		if (map.empty()) {
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
				.text = "No " + std::string(name) + " loaded",
				.color = Silica::GetTheme().Text_Dim
			}) });
		}
		else {
			for (const auto& [handle, asset] : map) {
				auto assetContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = ASSET_ITEM_SPACING });

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

				auto assetHeader = Silica::MakeWidget<Silica::SCollapsingHeader>({
					.title = std::string(name) + " [" + handle.uuid.toString() + "]",
					.initiallyOpen = false,
					.content = Silica::MakeWidget<Silica::SBox>({
						.padding = { INNER_HEADER_PAD_X, INNER_HEADER_PAD_Y },
						.child = assetContent
					}),
				});

				contentBox->addSlot({ {0,0}, assetHeader });
			}
		}

		return Silica::MakeWidget<Silica::SCollapsingHeader>({
			.title = label,
			.initiallyOpen = false,
			.content = Silica::MakeWidget<Silica::SBox>({
				.padding = { OUTER_HEADER_PAD_X, OUTER_HEADER_PAD_Y },
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

		// ----- Refresh Button -----
		auto refreshButton = Silica::MakeWidget<Silica::SButton>({
			.padding = { BUTTON_PAD_X, BUTTON_PAD_Y },
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				refresh();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Refresh Assets" })
		});

		// ----- Toolbar -----
		auto toolbar = Silica::MakeWidget<Silica::SBox>({
			.padding = { TOOLBAR_PADDING, TOOLBAR_PADDING },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.slots = {
					{ {2,0}, refreshButton }
				}
			})
		});


		// ----- Scrollable Content -----
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = SECTION_SPACING });

		// -- Mesh Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Mesh>("Mesh" , [&](Ref<Mesh> mesh) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Vertices", std::to_string(mesh->getVertexBuffer()->getVertexCount())) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Indices", std::to_string(mesh->getIndexCount())) });
			return box;
		}) });

		// -- Texture2D Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Texture2D>("Texture2D" , [&](Ref<Texture2D> tex) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Width", std::to_string(tex->getWidth()) + " px") });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Height", std::to_string(tex->getHeight()) + " px") });
			return box;
		}) });

		// -- TextureCube Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<TextureCube>("TextureCube" , [&](Ref<TextureCube> cube) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Face Width", std::to_string(cube->getFaceWidth()) + " px") });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Face Height", std::to_string(cube->getFaceHeight()) + " px") });
			return box;
		}) });

		// -- Material Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Material>("Material" , [&](Ref<Material> material) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Name", material->getName()) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Pipeline", material->getPipelineHandle().isValid() ? material->getPipelineHandle().uuid.toString() : "Internal Default") });
			return box;
		}) });

		// -- Skybox Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Skybox>("Skybox" , [&](Ref<Skybox> skybox) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Texture UUID", skybox->getTextureHandle().uuid.toString()) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Pipeline UUID", skybox->getPipelineHandle().isValid() ? skybox->getPipelineHandle().uuid.toString() : "Internal Default") });
			return box;
		}) });

		// -- Shader Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Shader>("Shader" , [&](Ref<Shader> shader) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Name", shader->getName()) });
			return box;
		}) });

		// -- Pipeline Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Pipeline>("Pipeline" , [&](Ref<Pipeline> pipeline) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			const auto& spec = pipeline->getSpecification();
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Color Format", EnumUtils::toString(spec.colorFormat)) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Depth Test", spec.depthTest ? "Enabled" : "Disabled") });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Topology", EnumUtils::toString(spec.topology)) });
			return box;
		}) });

		// -- AudioClip Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<AudioClip>("AudioClip" , [&](Ref<AudioClip> clip) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("File", clip->getPath().string()) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Load Mode", EnumUtils::toString(clip->getMode())) });
			return box;
		}) });

		// -- PhysicsMaterial Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<PhysicsMaterial>("PhysicsMaterial" , [&](Ref<PhysicsMaterial> physMat) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Static Friction", std::to_string(physMat->staticFriction)) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Dynamic Friction", std::to_string(physMat->dynamicFriction)) });
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Restitution", std::to_string(physMat->restitution)) });
			return box;
		}) });

		// -- Prefab Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Prefab>("Prefab" , [&](Ref<Prefab> prefab) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = ROW_SPACING});
			box->addSlot({ {0,0}, SilicaHelpers::MakeDetailRow("Entity Nodes", std::to_string(prefab->getEntityNode().size())) });
			return box;
		}) });


		// ----- Assemble -----
		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = { SCROLL_CONTENT_PADDING, SCROLL_CONTENT_PADDING },
				.child = contentBox
			})
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = toolbar,
			.contentArea = scrollBox
		}));
	}

}
