#pragma once
#include "AxionEngine/Source/render/Pipeline.h"

namespace Axion {

	class DX12Pipeline : public Pipeline {
	public:

		DX12Pipeline(const PipelineSpecification& spec);
		~DX12Pipeline() override;

		void release() override;

		void bind() override;
		void unbind() override;

		const PipelineSpecification& getSpecification() const override { return m_specification; }

	private:

		PipelineSpecification m_specification;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

	};

}
