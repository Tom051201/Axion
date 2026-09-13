#pragma once

#include "AxionEngine/Source/core/Math.h"

#include "AxionStudio/Source/ui/ModalBase.h"

namespace Axion {

	class MaterialImportModal : public ModalBase {
	public:

		MaterialImportModal();

	protected:

		void buildContent(std::shared_ptr<Silica::SVerticalBox> contentBox) override;
		void validate() override;
		void onConfirm() override;

	private:

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

		void resetInputs();

	};

}
