#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <DirectXMath.h>

#include "AxionEngine/Source/core/Ref.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/graphics/Mesh.h"

namespace Axion {

	struct SkeletalVertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT2 texcoord;
		uint32_t boneIDs[4];
		float boneWeights[4];
	};

	struct Bone {
		std::string name; // TODO: maybe change this to not use a string if possible, but if this is the best choice just keep it
		int32_t parentIndex = -1;
		std::vector<int32_t> children;
		DirectX::XMMATRIX localBindTransform;
		DirectX::XMMATRIX inverseBindMatrix;
	};

	struct Skeleton {
		std::vector<Bone> bones;
		DirectX::XMMATRIX rootTransform = DirectX::XMMatrixIdentity();
	};

	struct SkeletalMeshData {
		std::vector<SkeletalVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Submesh> submeshes;
		Skeleton skeleton;
	};

	class SkeletalMesh : public RefCounted {
	public:

		SkeletalMesh(const SkeletalMeshData& meshData);
		~SkeletalMesh();

		void release();

		Ref<VertexBuffer> getVertexBuffer() const { return m_vertexBuffer; }
		Ref<IndexBuffer> getIndexBuffer() const { return m_indexBuffer; }
		uint32_t getVertexCount() const { return m_vertexBuffer->getVertexCount(); }
		uint32_t getIndexCount() const { return m_indexBuffer->getIndexCount(); }
		const std::vector<Submesh>& getSubmeshes() const { return m_submeshes; }
		const Skeleton& getSkeleton() const { return m_skeleton; }
		const std::vector<DirectX::XMMATRIX>& getBindPoseMatrices() const { return m_bindPoseMatrices; }

		const AABB& getBoundingBox() const { return m_boundingBox; }

		static Ref<SkeletalMesh> create(const SkeletalMeshData& meshData);

	private:

		Ref<VertexBuffer> m_vertexBuffer;
		Ref<IndexBuffer> m_indexBuffer;
		std::vector<Submesh> m_submeshes;
		Skeleton m_skeleton;
		std::vector<DirectX::XMMATRIX> m_bindPoseMatrices;
		AABB m_boundingBox;

		void calculateBindPose(const Bone* bone, DirectX::XMMATRIX parentTransform);
		void calculateBounds(const std::vector<SkeletalVertex>& vertices);

	};


}
