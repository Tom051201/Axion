#include "AssetManagerPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/core/EnumUtils.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Shader.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/scene/Skybox.h"
#include "AxionEngine/Source/scene/Prefab.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"

namespace Axion {

	AssetManagerPanel::AssetManagerPanel(const std::string& name) : Panel(name) {}

	AssetManagerPanel::~AssetManagerPanel() {
		shutdown();
	}

	void AssetManagerPanel::setup() {}

	void AssetManagerPanel::shutdown() {}

	void AssetManagerPanel::onGuiRender() {
		ImGui::Begin("Asset Manager Inspector");

		auto drawRow = [](const char* label, const std::string& value) {
			ImGui::TableNextRow();
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextDisabled("%s", label);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextWrapped("%s", value.c_str());
		};

		#define BEGIN_ASSET_TABLE(name) \
			if (ImGui::BeginTable(name, 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp)) { \
				ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 100.0f); \
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		#define END_ASSET_TABLE() ImGui::EndTable(); }

		// -- Mesh assets --
		drawAssetInfo<Mesh>("Mesh", [&](Ref<Mesh> mesh) {
			BEGIN_ASSET_TABLE("MeshTable")
				drawRow("Vertices", std::to_string(mesh->getVertexBuffer()->getVertexCount()));
				drawRow("Indices", std::to_string(mesh->getIndexCount()));
			END_ASSET_TABLE()
		});


		// -- Texture2D Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<Texture2D>("Texture2D", [&](Ref<Texture2D> tex) {
			BEGIN_ASSET_TABLE("Tex2DTable")
				drawRow("Width", std::to_string(tex->getWidth()) + " px");
				drawRow("Height", std::to_string(tex->getHeight()) + " px");
			END_ASSET_TABLE()
		});;


		// -- TextureCube Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<TextureCube>("TextureCube", [&](Ref<TextureCube> cube) {
			BEGIN_ASSET_TABLE("TexCubeTable")
				drawRow("Face Width", std::to_string(cube->getFaceHeight()) + " px");
				drawRow("Face Height", std::to_string(cube->getFaceHeight()) + " px");
			END_ASSET_TABLE()
		});


		// -- Material Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<Material>("Material", [&](Ref<Material> material) {
			BEGIN_ASSET_TABLE("MaterialTable")
				drawRow("Name", material->getName());
				if (material->getPipelineHandle().isValid()) drawRow("Pipeline UUID", material->getPipelineHandle().uuid.toString());
				else drawRow("Pipeline UUID", "Internal Default Pipeline");
			END_ASSET_TABLE()
		});


		// -- Skybox Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<Skybox>("Skybox", [&](Ref<Skybox> skybox) {
			BEGIN_ASSET_TABLE("SkyboxTable")
				drawRow("Texture UUID", skybox->getTextureHandle().uuid.toString());
				if (skybox->getPipelineHandle().isValid()) drawRow("Pipeline UUID", skybox->getPipelineHandle().uuid.toString());
				else drawRow("Pipeline UUID", "Internal Default Pipeline");
			END_ASSET_TABLE()
		});


		// -- Shader Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<Shader>("Shader", [&](Ref<Shader> shader) {
			BEGIN_ASSET_TABLE("ShaderTable")
				drawRow("Name", shader->getName());
			END_ASSET_TABLE()
		});


		// -- Pipeline Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<Pipeline>("Pipeline", [&](Ref<Pipeline> pipeline) {
			BEGIN_ASSET_TABLE("PipelineTable")
				const auto& spec = pipeline->getSpecification();
				drawRow("Color Format", EnumUtils::toString(spec.colorFormat));
				drawRow("Depth Test", spec.depthTest ? "Enabled" : "Disabled");
				drawRow("Topology", EnumUtils::toString(spec.topology));
			END_ASSET_TABLE()
		});


		// -- AudioClip Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<AudioClip>("AudioClip", [&](Ref<AudioClip> clip) {
			BEGIN_ASSET_TABLE("AudioTable")
				drawRow("File", clip->getPath().string());
				drawRow("Load Mode", EnumUtils::toString(clip->getMode()));
			END_ASSET_TABLE()
		});


		// -- PhysicsMaterial Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<PhysicsMaterial>("PhysicsMaterial", [&](Ref<PhysicsMaterial> physMat) {
			BEGIN_ASSET_TABLE("PhysMatTable")
				drawRow("Static Friction", std::to_string(physMat->staticFriction));
				drawRow("Dynamic Friction", std::to_string(physMat->dynamicFriction));
				drawRow("Restitution", std::to_string(physMat->restitution));
			END_ASSET_TABLE()
		});

		// -- Prefab Assets --
		ImGui::SeparatorText("");
		drawAssetInfo<Prefab>("Prefab", [&](Ref<Prefab> prefab) {
			BEGIN_ASSET_TABLE("PrefabTable")
				drawRow("Entity Nodes", std::to_string(prefab->getEntityNode().size()));
			END_ASSET_TABLE()
		});

		#undef BEGIN_ASSET_TABLE
		#undef END_ASSET_TABLE

		ImGui::End();
	}

}
