#include "axpch.h"
#include "RendererAPI.h"

#include "AxionEngine/Platform/directx/D12RendererAPI.h"

namespace Axion {

	RendererAPI::API RendererAPI::s_api = RendererAPI::API::Direct3D12;

}
