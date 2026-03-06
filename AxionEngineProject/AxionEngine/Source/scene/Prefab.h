#pragma once

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

namespace Axion {

	class Prefab {
	public:

		Prefab() = default;
		Prefab(const YAML::Node& entityNode) : m_entityNode(entityNode) {}

		YAML::Node getEntityNode() const { return m_entityNode; }

	private:

		YAML::Node m_entityNode;

	};

}
