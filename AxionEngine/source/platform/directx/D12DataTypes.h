#pragma once
#include "axpch.h"

#include "Axion/render/DataTypes.h"

namespace Axion {

	struct D12Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texCoord;

		D12Vertex(const Vertex& v) :
			position(v.position[0], v.position[1], v.position[2]),
			color(v.color[0], v.color[1], v.color[2], v.color[3]),
			texCoord(v.texCoords[0], v.texCoords[1])
		{}

	};

}
