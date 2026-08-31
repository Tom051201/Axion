#include "NetworkManager.h"

#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>

#include "AxionEngine/Source/core/Logging.h"

namespace Axion::AXNetwork {

	void NetworkManager::initialize() {
		if (s_initialized) return;

		SteamNetworkingErrMsg errorMsg;

		if (!GameNetworkingSockets_Init(nullptr, errorMsg)) {
			AX_CORE_LOG_FATAL("Failed to initialize GameNetworkingSockets: {}", errorMsg);
			return;
		}

		s_initialized = true;
		AX_CORE_LOG_INFO("GameNetworkingSockets Initialized Successfully!");
	}

	void NetworkManager::shutdown() {
		if (!s_initialized) return;

		GameNetworkingSockets_Kill();
		s_initialized = false;
		AX_CORE_LOG_INFO("GameNetworkingSockets Shutdown");
	}

}
