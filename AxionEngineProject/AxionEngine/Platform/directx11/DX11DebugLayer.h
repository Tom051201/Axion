#pragma once

#include <d3d11.h>

namespace Axion {

	class DX11DebugLayer {
	public:

		static void initialize(ID3D11Device* device);
		static void reportLiveObjects(ID3D11Device* device);

		static void setName(ID3D11DeviceChild* object, const std::string& name);

	};

}
