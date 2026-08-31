#pragma once

#include <cstdint>

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/UUID.h"

namespace Axion::AXNetwork {

	enum class PacketType : uint16_t {
		None = 0,
		SpawnEntity,
		UpdateTransform,
		NetworkEvent
	};

	enum class EngineNetworkEvent : uint32_t {
		None = 0,
		PlayAudio3D = 1,
		PlayAnimation = 2,
		SpawnParticles = 3,

		CustomGameEventOffset = 1000
	};




	#pragma pack(push, 1)

	struct PacketHeader {
		PacketType type = PacketType::None;
	};

	struct SpawnEntityPacket {
		PacketHeader header{ PacketType::SpawnEntity };
		UUID networkID;
		Vec3 position;
		Quat rotation;
		Vec3 scale;
	};

	struct UpdateTransformPacket {
		PacketHeader header{ PacketType::UpdateTransform };
		UUID networkID;
		Vec3 position;
		Quat rotation;
		Vec3 scale;
	};

	struct NetworkEventPacket {
		PacketHeader header{ PacketType::NetworkEvent };
		UUID networkID;
		uint32_t eventID;
		uint16_t payloadSize;
		uint8_t payload[128];
	};

	#pragma pack(pop)

}
