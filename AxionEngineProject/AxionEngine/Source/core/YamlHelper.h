#pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/UUID.h"

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

namespace YAML {

	// ----- Vec3 -----
	template<>
	struct convert<Axion::Vec3> {

		static Node encode(const Axion::Vec3& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, Axion::Vec3& rhs) {
			if (!node.IsSequence() || node.size() != 3) return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}

	};


	// ----- Vec4 -----
	template<>
	struct convert<Axion::Vec4> {

		static Node encode(const Axion::Vec4& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, Axion::Vec4& rhs) {
			if (!node.IsSequence() || node.size() != 4) return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}

	};


	// ----- Quat -----
	template<>
	struct convert<Axion::Quat> {

		static Node encode(const Axion::Quat& rhs) {
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, Axion::Quat& rhs) {
			if (!node.IsSequence() || node.size() != 4) return false;
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}

	};


	// ----- UUID -----
	template<>
	struct convert<Axion::UUID> {

		static Node encode(const Axion::UUID& rhs) {
			return Node(rhs.toString());
		}

		static bool decode(const Node& node, Axion::UUID& rhs) {
			if (!node.IsScalar()) return false;
			try {
				rhs = Axion::UUID::fromString(node.as<std::string>());
				return true;
			}
			catch (...) {
				return false;
			}
		}

	};

}



namespace Axion {

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Vec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Vec4& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const Quat& q) {
		out << YAML::Flow;
		out << YAML::BeginSeq << q.x << q.y << q.z << q.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const UUID& uuid) {
		out << uuid.toString();
		return out;
	}

}
