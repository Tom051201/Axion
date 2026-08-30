#pragma once

namespace Axion {

	class DiscordManager {
	public:

		static void initialize(const std::string& applicationID);
		static void shutdown();

		static void onUpdate();

		static void setPresence(const std::string& state, const std::string& details, const std::string& largeImageKey = "axion_logo", const std::string& largeImageText = "Axion Engine");
		static void clearPresence();

	private:

		static inline int64_t s_startTimestamp = 0;
		static inline bool s_initialized = false;

	};

}
