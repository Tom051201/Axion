#include "axpch.h"
#include "RendererAPI.h"

#include "platform/directx/D12RendererAPI.h"

namespace Axion {

	RendererAPI::API RendererAPI::s_api = RendererAPI::API::Direct3D12;

}
