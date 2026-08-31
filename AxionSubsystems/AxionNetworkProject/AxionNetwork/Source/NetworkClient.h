#pragma once

#include <string>

#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>

#include "AxionEngine/Source/core/UUID.h"

namespace Axion {
	class Scene;
}

namespace Axion::AXNetwork {

	class NetworkClient {
	public:

		NetworkClient();
		~NetworkClient();

		bool connect(const std::string& ip, uint16_t port);
		void disconnect();
		void onUpdate(Scene* scene);

		void sendString(const std::string& message);
		void sendNetworkEvent(UUID entityID, uint32_t eventID, uint8_t* payload = nullptr, uint16_t payloadSize = 0);

	private:

		ISteamNetworkingSockets* m_interface = nullptr;
		HSteamNetConnection m_connection = k_HSteamNetConnection_Invalid;

		static void onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);
		static NetworkClient* s_instance;

	};

}
