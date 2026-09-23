#pragma once

#include <string>
#include <array>
#include <vector>
#include <cstdint>

#include <yaml-cpp/yaml.h>

namespace Axion {

	class PhysicsLayerManager {
	public:

		static void initialize();
		static void shutdown();

		static void setLayerName(uint32_t index, const std::string& name);
		static const std::string& getLayerName(uint32_t index);
		static const std::array<std::string, 32>& getLayers();

		static uint32_t getLayerIndexFromBitmask(uint32_t bitmask);
		static uint32_t getBitmaskFromLayerIndex(uint32_t index);

		static std::vector<std::string> getActiveLayerNames();
		static std::vector<uint32_t> getActiveLayerIndices();

		static void serialize(YAML::Emitter& out);
		static void deserialize(const YAML::Node& node);

		static void serializeBinary(std::ofstream& out);
		static void deserializeBinary(std::istream& in);

	private:

		static std::array<std::string, 32> s_layerNames;

	};

}
