#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <GameNetworkingSockets/steam/isteamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>

namespace Axion {
	class Scene;
	class Entity;
}

namespace Axion::AXNetwork {

	class NetworkServer {
	public:

		NetworkServer();
		~NetworkServer();

		bool start(uint16_t port);
		void stop();
		void onUpdate(Scene* scene);

		void spawnNetworkEntity(Entity entity);

	private:

		ISteamNetworkingSockets* m_interface = nullptr;
		HSteamListenSocket m_listenSocket = k_HSteamListenSocket_Invalid;
		HSteamNetPollGroup m_pollGroup = k_HSteamNetPollGroup_Invalid;

		std::vector<HSteamNetConnection> m_connectedClients;

		static void onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);

		static NetworkServer* s_instance;

	};

}
