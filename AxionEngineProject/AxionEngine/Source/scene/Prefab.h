#pragma once

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

namespace Axion {

	class Prefab {
	public:

		Prefab() = default;
		Prefab(const YAML::Node& entityNode) : m_entityNode(entityNode), m_isBinary(false) {}
		Prefab(std::vector<uint8_t>&& binaryData) : m_binaryData(binaryData), m_isBinary(true) {}

		bool isBinary() const { return m_isBinary; }
		YAML::Node getEntityNode() const { return m_entityNode; }
		const std::vector<uint8_t>& getBinaryData() const { return m_binaryData; }

	private:

		bool m_isBinary = false;
		YAML::Node m_entityNode;
		std::vector<uint8_t> m_binaryData;

	};

}
