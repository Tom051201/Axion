#pragma once

#include "AxionStudio/Source/core/Modal.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/MaterialData.h"

namespace Axion {

	class MaterialImportModal : public Modal {
	public:

		MaterialImportModal(const char* name);
		~MaterialImportModal() override;

		void close() override;

	private:

		void renderContent() override;

		void clearBuffers();

		char m_nameBuffer[128] = "";
		char m_sourcePathBuffer[256] = "";
		char m_outputPathBuffer[256] = "";

		Vec4 m_albedoColor = Vec4::one();
		float m_metalness = 0.0f;
		float m_roughness = 0.0f;
		float m_emission = 0.0f;
		float m_tiling = 0.0f;
		bool m_useNormalMap = false;
		bool m_useMetalnessMap = false;
		bool m_useRoughnessMap = false;
		bool m_useOcclusionMap = false;

		char m_albedoMapPathBuffer[256] = "";
		char m_normalMapPathBuffer[256] = "";
		char m_metalnessMapPathBuffer[256] = "";
		char m_roughnessMapPathBuffer[256] = "";
		char m_occlusionMapPathBuffer[256] = "";
		char m_emissiveMapPathBuffer[256] = "";

	};

}
