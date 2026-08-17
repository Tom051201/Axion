#include "axpch.h"
#include "SkeletalMesh.h"

namespace Axion {

	SkeletalMesh::SkeletalMesh(const SkeletalMeshData& meshData) {
		m_vertexBuffer = VertexBuffer::create(meshData.vertices);
		m_indexBuffer = IndexBuffer::create(meshData.indices);
		m_submeshes = meshData.submeshes;
		m_skeleton = meshData.skeleton;

		m_bindPoseMatrices.resize(m_skeleton.bones.size(), DirectX::XMMatrixIdentity());
		for (size_t i = 0; i < m_skeleton.bones.size(); ++i) {
			if (m_skeleton.bones[i].parentIndex == -1) {
				calculateBindPose(&m_skeleton.bones[i], m_skeleton.rootTransform);
			}
		}
	}

	SkeletalMesh::~SkeletalMesh() {
		release();
	}

	void SkeletalMesh::release() {
		m_vertexBuffer->release();
		m_indexBuffer->release();
		m_submeshes.clear();
		m_skeleton = {};
	}

	Ref<SkeletalMesh> SkeletalMesh::create(const SkeletalMeshData& meshData) {
		return std::make_shared<SkeletalMesh>(meshData);
	}

	void SkeletalMesh::calculateBindPose(const Bone* bone, DirectX::XMMATRIX parentTransform) {
		DirectX::XMMATRIX globalTransform = bone->localBindTransform * parentTransform;

		int boneIndex = static_cast<int>(bone - &m_skeleton.bones[0]);
		m_bindPoseMatrices[boneIndex] = bone->inverseBindMatrix * globalTransform;

		for (int childIndex : bone->children) {
			calculateBindPose(&m_skeleton.bones[childIndex], globalTransform);
		}
	}

}
