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

// ----- HELPER FUNCTIONS -----
namespace {

	Silica::WidgetPtr MakeRow(const std::string& label, const std::string& value) {
		return Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 15.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2{140.0f, 0.0f},
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::STextBlock>({
						.text = label,
						.color = Silica::GetTheme().Text_Dim
					})
				})},
				{ {1,0}, Silica::MakeWidget<Silica::STextBlock>({ .text = value }) }
			}
		});
	};



	template<typename T>
	Silica::WidgetPtr buildAssetInfoWidget(const char* name, std::function<Silica::WidgetPtr(Axion::Ref<T>)> elementFunc) {
		const auto& map = Axion::AssetManager::getMap<T>();
		std::string label = std::string(name) + " (" + std::to_string(map.size()) + ")";

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 6.0f });

		if (map.empty()) {
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
				.text = "No " + std::string(name) + " loaded",
				.color = Silica::GetTheme().Text_Dim
			}) });
		}
		else {
			for (const auto& [handle, asset] : map) {
				auto assetContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 4.0f });

				std::string filePath = Axion::AssetManager::getRelativeToAssets(Axion::AssetManager::getAssetFilePath<T>(handle)).string();
				assetContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({ .text = "Asset File: " + filePath }) });

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
						.padding = { 10.0f, 5.0f },
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
				.padding = { 15.0f, 5.0f },
				.child = contentBox
			}),
		});
	}

}





namespace Axion {

	Silica::WidgetPtr AssetManagerPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = 3.0f
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
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
			.padding = { 10.0f, 6.0f },
			//.color = Silica::GetTheme().Accent_Primary,
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				refresh();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({ .text = "Refresh Assets" })
		});

		// ----- Toolbar -----
		auto toolbar = Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.slots = {
					{ {2,0}, refreshButton }
				}
			})
		});


		// ----- Scrollable Content -----
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 10.0f });

		// -- Mesh Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Mesh>("Mesh" , [&](Ref<Mesh> mesh) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Vertices", std::to_string(mesh->getVertexBuffer()->getVertexCount()) ) });
			box->addSlot({ {0,0}, MakeRow("Indices", std::to_string(mesh->getIndexCount()) ) });
			return box;
		}) });

		// -- Texture2D Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Texture2D>("Texture2D" , [&](Ref<Texture2D> tex) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Width", std::to_string(tex->getWidth()) + " px" ) });
			box->addSlot({ {0,0}, MakeRow("Height", std::to_string(tex->getHeight()) + " px" ) });
			return box;
		}) });

		// -- TextureCube Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<TextureCube>("TextureCube" , [&](Ref<TextureCube> cube) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Face Width", std::to_string(cube->getFaceWidth()) + " px" ) });
			box->addSlot({ {0,0}, MakeRow("Face Height", std::to_string(cube->getFaceHeight()) + " px" ) });
			return box;
		}) });

		// -- Material Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Material>("Material" , [&](Ref<Material> material) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Name", material->getName() ) });
			box->addSlot({ {0,0}, MakeRow("Pipeline", material->getPipelineHandle().isValid() ? material->getPipelineHandle().uuid.toString() : "Internal Default" ) });
			return box;
		}) });

		// -- Skybox Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Skybox>("Skybox" , [&](Ref<Skybox> skybox) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Texture UUID", skybox->getTextureHandle().uuid.toString() ) });
			box->addSlot({ {0,0}, MakeRow("Pipeline UUID", skybox->getPipelineHandle().isValid() ? skybox->getPipelineHandle().uuid.toString() : "Internal Default" ) });
			return box;
		}) });

		// -- Shader Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Shader>("Shader" , [&](Ref<Shader> shader) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Name", shader->getName() ) });
			return box;
		}) });

		// -- Pipeline Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Pipeline>("Pipeline" , [&](Ref<Pipeline> pipeline) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			const auto& spec = pipeline->getSpecification();
			box->addSlot({ {0,0}, MakeRow("Color Format", EnumUtils::toString(spec.colorFormat) ) });
			box->addSlot({ {0,0}, MakeRow("Depth Test", spec.depthTest ? "Enabled" : "Disabled" ) });
			box->addSlot({ {0,0}, MakeRow("Topology", EnumUtils::toString(spec.topology) ) });
			return box;
		}) });

		// -- AudioClip Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<AudioClip>("AudioClip" , [&](Ref<AudioClip> clip) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("File", clip->getPath().string() ) });
			box->addSlot({ {0,0}, MakeRow("Load Mode", EnumUtils::toString(clip->getMode()) ) });
			return box;
		}) });

		// -- PhysicsMaterial Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<PhysicsMaterial>("PhysicsMaterial" , [&](Ref<PhysicsMaterial> physMat) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Static Friction", std::to_string(physMat->staticFriction) ) });
			box->addSlot({ {0,0}, MakeRow("Dynamic Friction", std::to_string(physMat->dynamicFriction) ) });
			box->addSlot({ {0,0}, MakeRow("Restitution", std::to_string(physMat->restitution) ) });
			return box;
		}) });

		// -- Prefab Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Prefab>("Prefab" , [&](Ref<Prefab> prefab) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Entity Nodes", std::to_string(prefab->getEntityNode().size()) ) });
			return box;
		}) });


		// ----- Assemble -----
		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = {10.0f, 10.0f},
				.child = contentBox
			})
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = toolbar,
			.contentArea = scrollBox
		}));
	}

}
