#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "AxionEngine/Source/graphics/Pipeline.h"

namespace Axion {

	class DX11Pipeline : public Pipeline {
	public:

		DX11Pipeline(const PipelineSpecification& spec);
		~DX11Pipeline() override;

		void release() override;

		void bind() override;
		void unbind() override;

		const PipelineSpecification& getSpecification() const override { return m_specification; }

	private:

		PipelineSpecification m_specification;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

		D3D11_PRIMITIVE_TOPOLOGY m_topology;

	};

}
