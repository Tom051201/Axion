#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/MaterialData.h"

#include "AxionStudio/Vendor/Silica/include/SWidget.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"
#include "AxionStudio/Vendor/Silica/include/FontAtlas.h"

#include <string>
#include <filesystem>
#include <functional>

namespace Axion {

	class MaterialImportModal {
	public:

		MaterialImportModal() { resetInputs(); }
		~MaterialImportModal() = default;

		Silica::WidgetPtr getWidget(Silica::FontAtlas* font, std::function<void()> onClose);

	private:

		void rebuildUI();
		void rebuildUI_Internal();
		void resetInputs();

		std::string m_name;
		std::string m_pipelinePath;
		std::string m_outputPath;

		Vec4 m_albedoColor = Vec4::one();
		float m_metalness = 0.0f;
		float m_roughness = 0.0f;
		float m_emission = 0.0f;
		float m_tiling = 0.0f;

		std::string m_albedoMapPath;
		std::string m_normalMapPath;
		std::string m_metalnessMapPath;
		std::string m_roughnessMapPath;
		std::string m_occlusionMapPath;
		std::string m_emissiveMapPath;

		// -- Silica --
		std::shared_ptr<Silica::SBox> m_uiRoot;
		Silica::FontAtlas* m_font = nullptr;
		std::function<void()> m_onClose;
		bool m_rebuildQueued = false;

	};

}
