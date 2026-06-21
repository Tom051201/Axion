#include "AssetManagerPanel.h"

#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"

#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

// ----- HELPER FUNCTIONS -----
namespace {

	Silica::WidgetPtr MakeRow(const std::string& label, const std::string& value, Silica::FontAtlas* font) {
		return Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 15.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2{140.0f, 0.0f},
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = label, .color = Silica::Color(150, 150, 150, 255), .font = font})
				})},
				{ {1,0}, Silica::MakeWidget<Silica::STextBlock>({.text = value, .font = font}) }
			}
		});
	};



	template<typename T>
	Silica::WidgetPtr buildAssetInfoWidget(const char* name, Silica::FontAtlas* font, std::function<Silica::WidgetPtr(Axion::Ref<T>)> elementFunc) {
		const auto& map = Axion::AssetManager::getMap<T>();
		std::string label = std::string(name) + " (" + std::to_string(map.size()) + ")";

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 6.0f });

		if (map.empty()) {
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
				.text = "No " + std::string(name) + " loaded",
				.color = Silica::Color(150, 150, 150, 255),
				.font = font
			}) });
		}
		else {
			for (const auto& [handle, asset] : map) {
				auto assetContent = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 4.0f });

				std::string filePath = Axion::AssetManager::getRelativeToAssets(Axion::AssetManager::getAssetFilePath<T>(handle)).string();
				assetContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
					.text = "Asset File: " + filePath,
					.color = Silica::Color(200, 200, 200, 255),
					.font = font
				}) });

				if (asset) {
					assetContent->addSlot({ {0,0}, elementFunc(asset) });
				}
				else {
					assetContent->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
						.text = std::string(name) + " data not loaded",
						.color = Silica::Color(255, 100, 100, 255),
						.font = font
					}) });
				}

				auto assetHeader = Silica::MakeWidget<Silica::SCollapsingHeader>({
					.title = std::string(name) + " [" + handle.uuid.toString() + "]",
					.initiallyOpen = false,
					.font = font,
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
			.font = font,
			.content = Silica::MakeWidget<Silica::SBox>({
				.padding = { 15.0f, 5.0f },
				.child = contentBox
			}),
		});
	}

}





namespace Axion {

	Silica::WidgetPtr AssetManagerPanel::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.backgroundColor = Silica::Color(25, 25, 25, 255)
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

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 10.0f });

		// ----- Refresh Button -----
		auto refreshButton = Silica::MakeWidget<Silica::SButton>({
			.padding = { 10.0f, 6.0f },
			.color = Silica::Color(40, 100, 150, 255),
			.hoverColor = Silica::Color(60, 130, 180, 255),
			.onClick = [this]() {
				refresh();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Refresh Assets", .font = m_font})
		});


		// ----- Top Toolbar -----
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SHorizontalBox>({
			.slots = {
				{ {0,0}, refreshButton }
			}
		}) });

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0,2}, .backgroundColor = Silica::Color(60,60,60,255)}) });

		// -- Mesh Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Mesh>("Mesh", m_font, [&](Ref<Mesh> mesh) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Vertices", std::to_string(mesh->getVertexBuffer()->getVertexCount()), m_font) });
			box->addSlot({ {0,0}, MakeRow("Indices", std::to_string(mesh->getIndexCount()), m_font) });
			return box;
		}) });

		// -- Texture2D Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Texture2D>("Texture2D", m_font, [&](Ref<Texture2D> tex) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Width", std::to_string(tex->getWidth()) + " px", m_font) });
			box->addSlot({ {0,0}, MakeRow("Height", std::to_string(tex->getHeight()) + " px", m_font) });
			return box;
		}) });

		// -- TextureCube Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<TextureCube>("TextureCube", m_font, [&](Ref<TextureCube> cube) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Face Width", std::to_string(cube->getFaceWidth()) + " px", m_font) });
			box->addSlot({ {0,0}, MakeRow("Face Height", std::to_string(cube->getFaceHeight()) + " px", m_font) });
			return box;
		}) });

		// -- Material Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Material>("Material", m_font, [&](Ref<Material> material) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Name", material->getName(), m_font) });
			box->addSlot({ {0,0}, MakeRow("Pipeline", material->getPipelineHandle().isValid() ? material->getPipelineHandle().uuid.toString() : "Internal Default", m_font) });
			return box;
		}) });

		// -- Skybox Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Skybox>("Skybox", m_font, [&](Ref<Skybox> skybox) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Texture UUID", skybox->getTextureHandle().uuid.toString(), m_font) });
			box->addSlot({ {0,0}, MakeRow("Pipeline UUID", skybox->getPipelineHandle().isValid() ? skybox->getPipelineHandle().uuid.toString() : "Internal Default", m_font) });
			return box;
		}) });

		// -- Shader Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Shader>("Shader", m_font, [&](Ref<Shader> shader) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Name", shader->getName(), m_font) });
			return box;
		}) });

		// -- Pipeline Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Pipeline>("Pipeline", m_font, [&](Ref<Pipeline> pipeline) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			const auto& spec = pipeline->getSpecification();
			box->addSlot({ {0,0}, MakeRow("Color Format", EnumUtils::toString(spec.colorFormat), m_font) });
			box->addSlot({ {0,0}, MakeRow("Depth Test", spec.depthTest ? "Enabled" : "Disabled", m_font) });
			box->addSlot({ {0,0}, MakeRow("Topology", EnumUtils::toString(spec.topology), m_font) });
			return box;
		}) });

		// -- AudioClip Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<AudioClip>("AudioClip", m_font, [&](Ref<AudioClip> clip) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("File", clip->getPath().string(), m_font) });
			box->addSlot({ {0,0}, MakeRow("Load Mode", EnumUtils::toString(clip->getMode()), m_font) });
			return box;
		}) });

		// -- PhysicsMaterial Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<PhysicsMaterial>("PhysicsMaterial", m_font, [&](Ref<PhysicsMaterial> physMat) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Static Friction", std::to_string(physMat->staticFriction), m_font) });
			box->addSlot({ {0,0}, MakeRow("Dynamic Friction", std::to_string(physMat->dynamicFriction), m_font) });
			box->addSlot({ {0,0}, MakeRow("Restitution", std::to_string(physMat->restitution), m_font) });
			return box;
		}) });

		// -- Prefab Assets --
		contentBox->addSlot({ {0,0}, buildAssetInfoWidget<Prefab>("Prefab", m_font, [&](Ref<Prefab> prefab) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 2.0f});
			box->addSlot({ {0,0}, MakeRow("Entity Nodes", std::to_string(prefab->getEntityNode().size()), m_font) });
			return box;
		}) });

		// ----- Assemble -----
		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = {10.0f, 10.0f},
				.child = contentBox
			})
		});

		m_uiRoot->setChild(scrollBox);
	}

}
