#include "axpch.h"
#include "PhysicsLayerManager.h"

#include <intrin.h>

namespace Axion {

	std::array<std::string, 32> PhysicsLayerManager::s_layerNames;

	void PhysicsLayerManager::initialize() {
		for (auto& name : s_layerNames) name = "";
		s_layerNames[0] = "Default";
		s_layerNames[1] = "Player";
		s_layerNames[2] = "Enemy";
		s_layerNames[3] = "Environment";
	}

	void PhysicsLayerManager::shutdown() {

	}

	void PhysicsLayerManager::setLayerName(uint32_t index, const std::string& name) {
		if (index < 32) s_layerNames[index] = name;
	}

	const std::string& PhysicsLayerManager::getLayerName(uint32_t index) {
		static std::string empty = "";
		if (index >= 32) return empty;
		return s_layerNames[index];
	}

	const std::array<std::string, 32>& PhysicsLayerManager::getLayers() {
		return s_layerNames;
	}

	uint32_t PhysicsLayerManager::getLayerIndexFromBitmask(uint32_t bitmask) {
		if (bitmask == 0) return 0;
		unsigned long index;
		_BitScanForward(&index, bitmask);
		return static_cast<uint32_t>(index);
	}

	uint32_t PhysicsLayerManager::getBitmaskFromLayerIndex(uint32_t index) {
		if (index >= 32) return 1;
		return 1 << index;
	}

	std::vector<std::string> PhysicsLayerManager::getActiveLayerNames() {
		std::vector<std::string> active;
		for (const auto& name : s_layerNames) {
			if (!name.empty()) active.push_back(name);
		}
		return active;
	}

	std::vector<uint32_t> PhysicsLayerManager::getActiveLayerIndices() {
		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < 32; i++) {
			if (!s_layerNames[i].empty()) indices.push_back(i);
		}
		return indices;
	}

	void PhysicsLayerManager::serialize(YAML::Emitter& out) {
		out << YAML::Key << "PhysicsLayers" << YAML::BeginMap;
		for (uint32_t i = 0; i < 32; i++) {
			if (!s_layerNames[i].empty()) {
				out << YAML::Key << std::to_string(i) << YAML::Value << s_layerNames[i];
			}
		}
		out << YAML::EndMap;
	}

	void PhysicsLayerManager::deserialize(const YAML::Node& node) {
		if (!node) return;
		for (auto& name : s_layerNames) name = "";

		for (auto it = node.begin(); it != node.end(); ++it) {
			uint32_t index = std::stoul(it->first.as<std::string>());
			if (index < 32) {
				s_layerNames[index] = it->second.as<std::string>();
			}
		}

		if (s_layerNames[0].empty()) s_layerNames[0] = "Default";
	}

	void PhysicsLayerManager::serializeBinary(std::ofstream& out) {
		for (uint32_t i = 0; i < 32; i++) {
			uint32_t len = static_cast<uint32_t>(s_layerNames[i].size());
			out.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
			if (len > 0) {
				out.write(s_layerNames[i].data(), len);
			}
		}
	}

	void PhysicsLayerManager::deserializeBinary(std::istream& in) {
		for (uint32_t i = 0; i < 32; i++) {
			uint32_t len = 0;
			in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
			if (len > 0) {
				s_layerNames[i].resize(len);
				in.read(&s_layerNames[i][0], len);
			}
			else {
				s_layerNames[i] = "";
			}
		}
		if (s_layerNames[0].empty()) s_layerNames[0] = "Default";
	}

}
