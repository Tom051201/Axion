#pragma once
#include "axpch.h"

namespace Axion {

	struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;

		Vertex()
			: position(0.0f, 0.0f, 0.0f), normal(0.0f, 0.0f, 1.0f), texcoord(0.0f, 0.0f) {}

		Vertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
			: position(px, py, pz), normal(nx, ny, nz), texcoord(u, v) {}

		Vertex(float px, float py, float pz)
			: position(px, py, pz), normal(0, 0, 0), texcoord(0, 0) {}

		static void normalizeVertices(std::vector<Vertex>& vertices) {
			if (vertices.empty()) return;

			// Find bounding box
			DirectX::XMFLOAT3 min = vertices[0].position;
			DirectX::XMFLOAT3 max = vertices[0].position;

			for (auto& v : vertices) {
				if (v.position.x < min.x) min.x = v.position.x;
				if (v.position.y < min.y) min.y = v.position.y;
				if (v.position.z < min.z) min.z = v.position.z;

				if (v.position.x > max.x) max.x = v.position.x;
				if (v.position.y > max.y) max.y = v.position.y;
				if (v.position.z > max.z) max.z = v.position.z;
			}

			// Compute center and scale
			DirectX::XMFLOAT3 center{
				(min.x + max.x) * 0.5f,
				(min.y + max.y) * 0.5f,
				(min.z + max.z) * 0.5f
			};

			float scaleX = max.x - min.x;
			float scaleY = max.y - min.y;
			float scaleZ = max.z - min.z;
			float maxScale = std::max({ scaleX, scaleY, scaleZ });

			if (maxScale == 0) return;

			float uniformScale = 1.0f / maxScale; // scale to [-0.5,0.5]

			// Apply normalization
			for (auto& v : vertices) {
				v.position.x = (v.position.x - center.x) * uniformScale;
				v.position.y = (v.position.y - center.y) * uniformScale;
				v.position.z = (v.position.z - center.z) * uniformScale;
			}
		}

		bool operator==(const Vertex& other) const {
			return position.x == other.position.x &&
				position.y == other.position.y &&
				position.z == other.position.z &&
				normal.x == other.normal.x &&
				normal.y == other.normal.y &&
				normal.z == other.normal.z &&
				texcoord.x == other.texcoord.x &&
				texcoord.y == other.texcoord.y;
		}

		bool operator!=(const Vertex& other) const { return !(*this == other); }

	};

}

namespace std {

	template<>
	struct hash<Axion::Vertex> {
		size_t operator()(const Axion::Vertex& v) const noexcept {
			size_t h1 = std::hash<float>()(v.position.x) ^ (std::hash<float>()(v.position.y) << 1) ^ (std::hash<float>()(v.position.z) << 2);
			size_t h2 = std::hash<float>()(v.normal.x) ^ (std::hash<float>()(v.normal.y) << 1) ^ (std::hash<float>()(v.normal.z) << 2);
			size_t h3 = std::hash<float>()(v.texcoord.x) ^ (std::hash<float>()(v.texcoord.y) << 1);
			return h1 ^ (h2 << 1) ^ (h3 << 2);
		}
	};

}
