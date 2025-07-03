#pragma once

#include "AxionEngine/Source/core/Core.h"

#include "AxionEngine/Vendor/spdlog/include/spdlog/spdlog.h"
#include "AxionEngine/Vendor/spdlog/include/spdlog/fmt/ostr.h"

namespace Axion {

	class Log {
	public:

		static void init();

		inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_coreLogger; }
		inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_clientLogger; }
	
	private:

		static std::shared_ptr<spdlog::logger> s_coreLogger;
		static std::shared_ptr<spdlog::logger> s_clientLogger;
	
	};

}

#ifdef AX_DEBUG

	#define AX_CORE_LOG_TRACE(...)	::Axion::Log::getCoreLogger()->trace(__VA_ARGS__)
	#define AX_CORE_LOG_INFO(...)	::Axion::Log::getCoreLogger()->info(__VA_ARGS__)
	#define AX_CORE_LOG_WARN(...)	::Axion::Log::getCoreLogger()->warn(__VA_ARGS__)
	#define AX_CORE_LOG_ERROR(...)	::Axion::Log::getCoreLogger()->error(__VA_ARGS__)
	#define AX_CORE_LOG_FATAL(...)	::Axion::Log::getCoreLogger()->critical(__VA_ARGS__)
	
	#define AX_LOG_TRACE(...)		::Axion::Log::getClientLogger()->trace(__VA_ARGS__)
	#define AX_LOG_INFO(...)		::Axion::Log::getClientLogger()->info(__VA_ARGS__)
	#define AX_LOG_WARN(...)		::Axion::Log::getClientLogger()->warn(__VA_ARGS__)
	#define AX_LOG_ERROR(...)		::Axion::Log::getClientLogger()->error(__VA_ARGS__)
	#define AX_LOG_FATAL(...)		::Axion::Log::getClientLogger()->critical(__VA_ARGS__)

#endif

#ifdef AX_RELEASE

	#define AX_CORE_LOG_TRACE(...)
	#define AX_CORE_LOG_INFO(...)	::Axion::Log::getCoreLogger()->info(__VA_ARGS__)
	#define AX_CORE_LOG_WARN(...)	::Axion::Log::getCoreLogger()->warn(__VA_ARGS__)
	#define AX_CORE_LOG_ERROR(...)	::Axion::Log::getCoreLogger()->error(__VA_ARGS__)
	#define AX_CORE_LOG_FATAL(...)	::Axion::Log::getCoreLogger()->critical(__VA_ARGS__)

	#define AX_LOG_TRACE(...)
	#define AX_LOG_INFO(...)		::Axion::Log::getClientLogger()->info(__VA_ARGS__)
	#define AX_LOG_WARN(...)		::Axion::Log::getClientLogger()->warn(__VA_ARGS__)
	#define AX_LOG_ERROR(...)		::Axion::Log::getClientLogger()->error(__VA_ARGS__)
	#define AX_LOG_FATAL(...)		::Axion::Log::getClientLogger()->critical(__VA_ARGS__)

#endif

#ifdef AX_DISTRIBUTION
	
	#define AX_CORE_LOG_TRACE(...)
	#define AX_CORE_LOG_INFO(...)
	#define AX_CORE_LOG_WARN(...)
	#define AX_CORE_LOG_ERROR(...)
	#define AX_CORE_LOG_FATAL(...)
	
	#define AX_LOG_TRACE(...)
	#define AX_LOG_INFO(...)
	#define AX_LOG_WARN(...)
	#define AX_LOG_ERROR(...)
	#define AX_LOG_FATAL(...)

#endif
