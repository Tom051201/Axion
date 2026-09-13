#include "studiopch.h"
#include "DiscordManager.h"

#include <discord_rpc.h>

#include "AxionEngine/Source/core/Logging.h"

namespace Axion {

	// ----- HELPER -----
	static void handleDiscordReady(const DiscordUser* request) {
		AX_CORE_LOG_INFO("Discord RPC: Connected to user {}#{}", request->username, request->discriminator);
	}

	static void handleDiscordDisconnected(int errorCode, const char* message) {
		AX_CORE_LOG_WARN("Discord RPC: Disconnected ({}): {}", errorCode, message);
	}

	static void handleDiscordError(int errorCode, const char* message) {
		AX_CORE_LOG_ERROR("Discord RPC: Error ({}): {}", errorCode, message);
	}



	// ----- IMPLEMENTATION -----
	void DiscordManager::initialize(const std::string& applicationID) {
		if (s_initialized) return;

		DiscordEventHandlers handlers;
		memset(&handlers, 0, sizeof(handlers));
		handlers.ready = handleDiscordReady;
		handlers.disconnected = handleDiscordDisconnected;
		handlers.errored = handleDiscordError;

		Discord_Initialize(applicationID.c_str(), &handlers, 1, nullptr);

		s_startTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();

		s_initialized = true;
		AX_CORE_LOG_INFO("Discord RPC Initialized");
	}

	void DiscordManager::shutdown() {
		if (!s_initialized) return;

		Discord_ClearPresence();
		Discord_RunCallbacks();
		Discord_Shutdown();
		s_initialized = false;
		AX_CORE_LOG_INFO("Discord RPC Shutdown");
	}

	void DiscordManager::onUpdate() {
		if (!s_initialized) return;

		Discord_RunCallbacks();
	}

	void DiscordManager::setPresence(const std::string & state, const std::string & details, const std::string & largeImageKey, const std::string & largeImageText) {
		if (!s_initialized) return;

		DiscordRichPresence presence;
		memset(&presence, 0, sizeof(presence));

		presence.state = state.c_str();
		presence.details = details.c_str();
		presence.startTimestamp = s_startTimestamp;
		presence.largeImageKey = largeImageKey.c_str();
		presence.largeImageText = largeImageText.c_str();

		Discord_UpdatePresence(&presence);
	}

	void DiscordManager::clearPresence() {
		if (!s_initialized) return;
		Discord_ClearPresence();
	}

}
