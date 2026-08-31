#include "NetworkClient.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/scene/Scene.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/Entity.h"

#include "AxionNetwork/Source/NetworkPackets.h"

namespace Axion::AXNetwork {

	NetworkClient* NetworkClient::s_instance = nullptr;

	NetworkClient::NetworkClient() {
		s_instance = this;
	}

	NetworkClient::~NetworkClient() {
		disconnect();
		if (s_instance == this) s_instance = nullptr;
	}

	bool NetworkClient::connect(const std::string& ip, uint16_t port) {
		m_interface = SteamNetworkingSockets();
		if (!m_interface) {
			AX_CORE_LOG_ERROR("NetworkClient: Failed to get ISteamNetworkingSockets interface!");
			return false;
		}

		// -- Parse IP address and port --
		SteamNetworkingIPAddr serverAddr;
		serverAddr.Clear();
		std::string fullAddress = ip + ":" + std::to_string(port);
		if (!serverAddr.ParseString(fullAddress.c_str())) {
			AX_CORE_LOG_ERROR("NetworkClient: Invalid IP address: {}", fullAddress);
			return false;
		}

		SteamNetworkingConfigValue_t opt;
		opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)onConnectionStatusChanged);

		AX_CORE_LOG_INFO("NetworkClient: Connecting to {}...", fullAddress);
		m_connection = m_interface->ConnectByIPAddress(serverAddr, 1, &opt);

		if (m_connection == k_HSteamNetConnection_Invalid) {
			AX_CORE_LOG_ERROR("NetworkClient: Failed to initiate connection!");
			return false;
		}

		return true;
	}

	void NetworkClient::disconnect() {
		if (m_interface && m_connection != k_HSteamNetConnection_Invalid) {
			m_interface->CloseConnection(m_connection, 0, "Client Disconnecting", true);
			m_connection = k_HSteamNetConnection_Invalid;
			AX_CORE_LOG_INFO("NetworkClient: Disconnected.");
		}
	}

	void NetworkClient::onUpdate(Scene* scene) {
		if (!m_interface || m_connection == k_HSteamNetConnection_Invalid) return;

		m_interface->RunCallbacks();

		ISteamNetworkingMessage* incomingMsgs[16];
		int numMsgs = m_interface->ReceiveMessagesOnConnection(m_connection, incomingMsgs, 16);

		for (int i = 0; i < numMsgs; ++i) {
			ISteamNetworkingMessage* msg = incomingMsgs[i];

			// -- Read the header --
			if (msg->m_cbSize >= sizeof(PacketHeader)) {
				PacketHeader* header = (PacketHeader*)msg->m_pData;

				// -- Update transform --
				if (header->type == PacketType::UpdateTransform && msg->m_cbSize == sizeof(UpdateTransformPacket)) {
					UpdateTransformPacket* packet = (UpdateTransformPacket*)msg->m_pData;

					if (scene) {
						Entity targetEntity = scene->getEntityByUUID(packet->networkID);
						if (targetEntity && targetEntity.hasComponent<NetworkIdentityComponent>()) {
							if (!targetEntity.getComponent<NetworkIdentityComponent>().isLocalPlayer) {
								auto& transform = targetEntity.getComponent<TransformComponent>();
								transform.position = packet->position;
								transform.rotation = packet->rotation;
								transform.scale = packet->scale;
							}
						}
					}
				}
				// -- Spawn entity --
				else if (header->type == PacketType::SpawnEntity && msg->m_cbSize == sizeof(SpawnEntityPacket)) {
					SpawnEntityPacket* packet = (SpawnEntityPacket*)msg->m_pData;

					if (scene) {
						if (!scene->getEntityByUUID(packet->networkID)) {

							Entity newEntity = scene->createEntityWithUUID("Network Player", packet->networkID);
							newEntity.addComponent<NetworkIdentityComponent>(0);

							auto& transform = newEntity.getComponent<TransformComponent>();
							transform.position = packet->position;
							transform.rotation = packet->rotation;
							transform.scale = packet->scale;

							AX_CORE_LOG_TRACE("NetworkClient: Spawned new networked entity!");
						}
					}
				}
				// -- Network event --
				else if (header->type == PacketType::NetworkEvent && msg->m_cbSize == sizeof(NetworkEventPacket)) {
					NetworkEventPacket* packet = (NetworkEventPacket*)msg->m_pData;

					if (scene) {
						Entity targetEntity = scene->getEntityByUUID(packet->networkID);
						if (targetEntity) {

							// -- Engine native rounting --
							if (packet->eventID < (uint32_t)EngineNetworkEvent::CustomGameEventOffset) {

								EngineNetworkEvent engineEvent = static_cast<EngineNetworkEvent>(packet->eventID);

								switch (engineEvent) {
									case EngineNetworkEvent::PlayAudio3D:
										// TODO
										break;
									case EngineNetworkEvent::PlayAnimation:
										// TODO
										break;
									case EngineNetworkEvent::SpawnParticles:
										// TODO
										break;
									default:
										break;
								}
							}
							// -- C# script routing
							else {
								if (targetEntity.hasComponent<ScriptComponent>()) {
									// Route directly to C# using ScriptEngine
									// ScriptEngine::onNetworkEvent(targetEntity, packet->eventID, packet->payload, packet->payloadSize);
									// TODO
								}
							}
						}
					}
				}
			}

			msg->Release();
		}

		// -- Broadcast local player state to Server --
		if (scene) {
			auto view = scene->getRegistry().view<NetworkIdentityComponent, TransformComponent, UUIDComponent>();

			for (auto [entityHandle, networkID, transform, uuid] : view.each()) {
				if (networkID.isLocalPlayer) {
					UpdateTransformPacket packet;
					packet.networkID = uuid.id;
					packet.position = transform.position;
					packet.rotation = transform.rotation;
					packet.scale = transform.scale;

					m_interface->SendMessageToConnection(m_connection, &packet, sizeof(UpdateTransformPacket), k_nSteamNetworkingSend_Unreliable, nullptr);
				}
			}
		}
	}

	void NetworkClient::sendString(const std::string& message) {
		if (!m_interface || m_connection == k_HSteamNetConnection_Invalid) return;

		m_interface->SendMessageToConnection( m_connection, message.c_str(), (uint32_t)message.length(), k_nSteamNetworkingSend_Reliable, nullptr);
	}

	void NetworkClient::sendNetworkEvent(UUID entityID, uint32_t eventID, uint8_t* payload, uint16_t payloadSize) {
		if (!m_interface || m_connection == k_HSteamNetConnection_Invalid) return;

		NetworkEventPacket packet;
		packet.networkID = entityID;
		packet.eventID = eventID;
		packet.payloadSize = std::min((uint16_t)128, payloadSize);

		if (payload && packet.payloadSize > 0) {
			memcpy(packet.payload, payload, packet.payloadSize);
		}

		m_interface->SendMessageToConnection(m_connection, &packet, sizeof(NetworkEventPacket), k_nSteamNetworkingSend_Reliable, nullptr);
	}

	void NetworkClient::onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) {
		if (!s_instance || !s_instance->m_interface) return;

		switch (info->m_info.m_eState) {
			case k_ESteamNetworkingConnectionState_Connected: {
				AX_CORE_LOG_INFO("NetworkClient: Successfully connected to server!");
				s_instance->sendString("Hello Server! This is AxionClient speaking.");
				break;
			}
			case k_ESteamNetworkingConnectionState_ClosedByPeer:
			case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
				AX_CORE_LOG_INFO("NetworkClient: Disconnected from server. Reason: {}", info->m_info.m_szEndDebug);
				s_instance->m_interface->CloseConnection(info->m_hConn, 0, nullptr, false);
				s_instance->m_connection = k_HSteamNetConnection_Invalid;
				break;
			}
		}
	}

}
