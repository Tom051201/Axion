#pragma once
#include <memory>

#ifdef AX_ENABLE_ASSERTS
	#define AX_ASSERT(x, ...) { if (!(x)) { AX_LOG_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define AX_CORE_ASSERT(x, ...) { if (!(x)) { AX_CORE_LOG_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define AX_ASSERT(x, ...)
	#define AX_CORE_ASSERT(x, ...)
#endif



#define BIT(x) (1 << x)

#define AX_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }



#ifdef AX_PLATFORM_WINDOWS
	#define AX_EVAL_HR(hr, error) if ((hr) != S_OK) { AX_CORE_LOG_ERROR("API Error: {0} at file: {1}", error, __FILE__); }
	#define AX_THROW_IF_FAILED_HR(hr, msg) if (FAILED(hr)) { AX_CORE_LOG_ERROR(msg); throw std::runtime_error("HRESULT failed"); }
#else
	#define AX_EVAL_HR(hr, error)
	#define AX_THROW_IF_FAILED_HR(hr, msg)
#endif

namespace Axion {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using WeakRef = std::weak_ptr<T>;

}
