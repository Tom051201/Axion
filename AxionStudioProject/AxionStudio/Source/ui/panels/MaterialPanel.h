#pragma once

#include <memory>
#include <string>
#include <filesystem>

#include <Silica/include/SWidget.h>
#include <Silica/include/Renderer.h>

#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/core/AssetHandle.h"
#include "AxionEngine/Source/graphics/Material.h"
#include "AxionEngine/Source/graphics/Framebuffer.h"
#include "AxionEngine/Source/graphics/Camera.h"
#include "AxionEngine/Source/graphics/Mesh.h"

namespace Silica {
	class SBox;
}

namespace Axion {

	class MaterialPanel {
	public:

		MaterialPanel();
		~MaterialPanel() = default;

		Silica::WidgetPtr getWidget();

		void onUpdate(Timestep ts);
		void setMaterial(const std::filesystem::path& materialPath);

	private:

		// -- UI Elements --
		std::shared_ptr<Silica::SBox> m_uiRoot;

		// -- Material Data --
		std::filesystem::path m_currentMaterialPath;
		AssetHandle<Material> m_materialHandle;
		Ref<Material> m_material;

		// -- Preview Rendering --
		Ref<FrameBuffer> m_previewFramebuffer;
		Silica::TextureID m_viewportTextureID = 0;
		enum class PreviewShape { Sphere, Cube, Custom };
		PreviewShape m_previewShape = PreviewShape::Cube;
		AssetHandle<Mesh> m_customMeshHandle;
		std::string m_customMeshName = "Drop Mesh";

		// -- Orbit Camera Variables --
		Camera m_previewCamera;
		float m_cameraPitch = 0.0f;
		float m_cameraYaw = 0.0f;
		float m_cameraDistance = 3.0f;

		void rebuildUI();
		Silica::WidgetPtr buildProperties();
		Silica::WidgetPtr buildTextureSlot(const std::string& label, TextureSlot slot);

	};

}
