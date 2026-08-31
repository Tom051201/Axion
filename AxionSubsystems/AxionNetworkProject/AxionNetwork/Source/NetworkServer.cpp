#include "NetworkServer.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/Entity.h"

#include "AxionNetwork/Source/NetworkPackets.h"

namespace Axion::AXNetwork {

	NetworkServer* NetworkServer::s_instance = nullptr;

	NetworkServer::NetworkServer() {
		s_instance = this;
	}

	NetworkServer::~NetworkServer() {
		stop();
		if (s_instance == this) s_instance = nullptr;
	}

	bool NetworkServer::start(uint16_t port) {
		m_interface = SteamNetworkingSockets();
		if (!m_interface) {
			AX_CORE_LOG_ERROR("NetworkServer: Failed to get ISteamNetworkingSockets interface!");
			return false;
		}

		// -- Setup local address and port --
		SteamNetworkingIPAddr serverLocalAddr;
		serverLocalAddr.Clear();
		serverLocalAddr.m_port = port;

		SteamNetworkingConfigValue_t opt;
		opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)onConnectionStatusChanged);

		m_listenSocket = m_interface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
		if (m_listenSocket == k_HSteamListenSocket_Invalid) {
			AX_CORE_LOG_ERROR("NetworkServer: Failed to listen on port {}", port);
			return false;
		}

		m_pollGroup = m_interface->CreatePollGroup();
		if (m_pollGroup == k_HSteamNetPollGroup_Invalid) {
			AX_CORE_LOG_ERROR("NetworkServer: Failed to create poll group!");
			return false;
		}

		AX_CORE_LOG_INFO("NetworkServer: Started listening on port {}", port);
		return true;
	}

	void NetworkServer::stop() {
		if (m_interface) {
			if (m_listenSocket != k_HSteamListenSocket_Invalid) {
				m_interface->CloseListenSocket(m_listenSocket);
				m_listenSocket = k_HSteamListenSocket_Invalid;
			}

			if (m_pollGroup != k_HSteamNetPollGroup_Invalid) {
				m_interface->DestroyPollGroup(m_pollGroup);
				m_pollGroup = k_HSteamNetPollGroup_Invalid;
			}

			// -- Disconnect all clients --
			for (auto conn : m_connectedClients) {
				m_interface->CloseConnection(conn, 0, "Server Shutdown", true);
			}
			m_connectedClients.clear();

		}
		AX_CORE_LOG_INFO("NetworkServer: Stopped.");
	}

	void NetworkServer::onUpdate(Scene* scene) {
		if (!m_interface || m_listenSocket == k_HSteamListenSocket_Invalid) return;

		m_interface->RunCallbacks();

		// -- Process incoming messages --
		ISteamNetworkingMessage* incomingMsgs[16];
		int numMsgs = m_interface->ReceiveMessagesOnPollGroup(m_pollGroup, incomingMsgs, 16);
		for (int i = 0; i < numMsgs; i++) {
			ISteamNetworkingMessage* msg = incomingMsgs[i];

			if (msg->m_cbSize >= sizeof(PacketHeader)) {
				PacketHeader* header = (PacketHeader*)msg->m_pData;

				if (header->type == PacketType::UpdateTransform && msg->m_cbSize == sizeof(UpdateTransformPacket)) {
					UpdateTransformPacket* packet = (UpdateTransformPacket*)msg->m_pData;

					if (scene) {
						Entity targetEntity = scene->getEntityByUUID(packet->networkID);
						if (targetEntity) {
							auto& transform = targetEntity.getComponent<TransformComponent>();
							transform.position = packet->position;
							transform.rotation = packet->rotation;
							transform.scale = packet->scale;
						}
					}
				}
				else if (header->type == PacketType::NetworkEvent && msg->m_cbSize == sizeof(NetworkEventPacket)) {
					NetworkEventPacket* packet = (NetworkEventPacket*)msg->m_pData;

					AX_CORE_LOG_TRACE("Server received Event {} for Entity {}", std::to_string(packet->eventID), packet->networkID.toString());

					// Validation checks here

					// -- Bounce event to all clients --
					for (auto conn : m_connectedClients) {
						if (conn != msg->m_conn) {
							m_interface->SendMessageToConnection(conn, packet, sizeof(NetworkEventPacket), k_nSteamNetworkingSend_Reliable, nullptr);
						}
					}
				}
			}

			msg->Release();
		}

		// -- Broadcast scene state to clients --
		if (scene && !m_connectedClients.empty()) {
			auto view = scene->getRegistry().view<NetworkIdentityComponent, TransformComponent, UUIDComponent>();

			for (auto [entityHandle, networkID, transform, uuid] : view.each()) {
				UpdateTransformPacket packet;
				packet.networkID = uuid.id;
				packet.position = transform.position;
				packet.rotation = transform.rotation;
				packet.scale = transform.scale;

				// -- Send packet unreliable --
				for (auto conn : m_connectedClients) {
					m_interface->SendMessageToConnection(conn, &packet, sizeof(UpdateTransformPacket), k_nSteamNetworkingSend_Unreliable, nullptr);
				}
			}
		}

	}

	void NetworkServer::spawnNetworkEntity(Entity entity) {
		if (m_connectedClients.empty() || !entity.hasComponent<NetworkIdentityComponent>()) return;

		auto& uuid = entity.getComponent<UUIDComponent>();
		auto& transform = entity.getComponent<TransformComponent>();

		SpawnEntityPacket packet;
		packet.networkID = uuid.id;
		packet.position = transform.position;
		packet.rotation = transform.rotation;
		packet.scale = transform.scale;

		// -- Send packet reliable --
		for (auto conn : m_connectedClients) {
			m_interface->SendMessageToConnection(conn, &packet, sizeof(SpawnEntityPacket), k_nSteamNetworkingSend_Reliable,nullptr);
		}
		AX_CORE_LOG_TRACE("NetworkServer: Broadcasted Spawn packet for Entity {}", uuid.id.toString());
	}

	void NetworkServer::onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) {
		if (!s_instance || !s_instance->m_interface) return;

		switch (info->m_info.m_eState) {
			case k_ESteamNetworkingConnectionState_Connecting: {
				AX_CORE_LOG_INFO("NetworkServer: Client connecting (ConnID: {})", info->m_hConn);

				// -- Accept the connection --
				if (s_instance->m_interface->AcceptConnection(info->m_hConn) != k_EResultOK) {
					s_instance->m_interface->CloseConnection(info->m_hConn, 0, "Failed to accept connection", false);
					break;
				}

				// -- Assign the connection to poll group --
				s_instance->m_interface->SetConnectionPollGroup(info->m_hConn, s_instance->m_pollGroup);
				break;
			}
			case k_ESteamNetworkingConnectionState_Connected: {
				AX_CORE_LOG_INFO("NetworkServer: Client connected successfully! (ConnID: {})", info->m_hConn);
				s_instance->m_connectedClients.push_back(info->m_hConn);
				break;
			}
			case k_ESteamNetworkingConnectionState_ClosedByPeer:
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
				AX_CORE_LOG_INFO("NetworkServer: Client disconnected (ConnID: {})", info->m_hConn);
				s_instance->m_interface->CloseConnection(info->m_hConn, 0, nullptr, false);

				// -- Remove from connected list --
				auto it = std::find(s_instance->m_connectedClients.begin(), s_instance->m_connectedClients.end(), info->m_hConn);
				if (it != s_instance->m_connectedClients.end()) {
					s_instance->m_connectedClients.erase(it);
				}
				break;
			}
		}
	}

}
