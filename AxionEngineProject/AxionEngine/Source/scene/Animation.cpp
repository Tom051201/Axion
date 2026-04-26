#include "axpch.h"
#include "Animation.h"

namespace Axion {

	const BoneAnimation* AnimationClip::getBoneAnimation(const std::string& name) const {
		for (const auto& anim : boneAnimations) {
			if (anim.boneName == name) return &anim;
		}
		return nullptr;
	}

	Animator::Animator(const Skeleton& skeleton)
		: m_skeleton(skeleton) {
		m_finalBoneMatrices.resize(100, DirectX::XMMatrixIdentity());
	}

	void Animator::playAnimation(const Ref<AnimationClip>& clip) {
		m_currentClip = clip;
		m_currentTime = 0.0f;
	}

	void Animator::update(Timestep ts) {
		if (m_skeleton.bones.empty()) return;

		if (m_currentClip) {
			m_currentTime += ts.getSeconds() * m_currentClip->ticksPerSecond;
			if (m_currentClip->duration > 0.0f) {
				m_currentTime = fmod(m_currentTime, m_currentClip->duration);
			}
			else {
				m_currentTime = 0.0f;
			}
		}

		for (size_t i = 0; i < m_skeleton.bones.size(); ++i) {
			if (m_skeleton.bones[i].parentIndex == -1) {
				calculateBoneTransform(&m_skeleton.bones[i], m_skeleton.rootTransform);
			}
		}
	}

	const std::vector<DirectX::XMMATRIX>& Animator::getFinalMatrices() const {
		return m_finalBoneMatrices;
	}

	template<typename T>
	int GetKeyframeIndex(const std::vector<Keyframe<T>>& keys, float time) {
		for (size_t i = 0; i < keys.size() - 1; ++i) {
			if (time < keys[i + 1].time) return static_cast<int>(i);
		}
		return 0;
	}

	void Animator::calculateBoneTransform(const Bone* bone, DirectX::XMMATRIX parentTransform) {
		DirectX::XMMATRIX localTransform = bone->localBindTransform;

		if (m_currentClip) {
			const BoneAnimation* boneAnim = m_currentClip->getBoneAnimation(bone->name);
			if (boneAnim) {
				DirectX::XMVECTOR S, R, T;
				DirectX::XMMatrixDecompose(&S, &R, &T, localTransform);

				// -- Interpolate Translation --
				if (!boneAnim->positions.empty()) {
					if (boneAnim->positions.size() == 1) {
						T = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&boneAnim->positions[0].value);
					}
					else {
						int idx = GetKeyframeIndex(boneAnim->positions, m_currentTime);
						int nextIdx = idx + 1;
						float factor = (m_currentTime - boneAnim->positions[idx].time) / (boneAnim->positions[nextIdx].time - boneAnim->positions[idx].time);

						DirectX::XMVECTOR start = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&boneAnim->positions[idx].value);
						DirectX::XMVECTOR end = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&boneAnim->positions[nextIdx].value);
						T = DirectX::XMVectorLerp(start, end, factor);
					}
				}

				// -- Interpolate Rotation --
				if (!boneAnim->rotations.empty()) {
					if (boneAnim->rotations.size() == 1) {
						R = DirectX::XMLoadFloat4((DirectX::XMFLOAT4*)&boneAnim->rotations[0].value);
					}
					else {
						int idx = GetKeyframeIndex(boneAnim->rotations, m_currentTime);
						int nextIdx = idx + 1;
						float factor = (m_currentTime - boneAnim->rotations[idx].time) / (boneAnim->rotations[nextIdx].time - boneAnim->rotations[idx].time);

						DirectX::XMVECTOR start = DirectX::XMLoadFloat4((DirectX::XMFLOAT4*)&boneAnim->rotations[idx].value);
						DirectX::XMVECTOR end = DirectX::XMLoadFloat4((DirectX::XMFLOAT4*)&boneAnim->rotations[nextIdx].value);
						R = DirectX::XMQuaternionSlerp(start, end, factor);
					}
				}

				// -- Interpolate Scale --
				if (!boneAnim->scales.empty()) {
					if (boneAnim->scales.size() == 1) {
						S = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&boneAnim->scales[0].value);
					}
					else {
						int idx = GetKeyframeIndex(boneAnim->scales, m_currentTime);
						int nextIdx = idx + 1;
						float factor = (m_currentTime - boneAnim->scales[idx].time) / (boneAnim->scales[nextIdx].time - boneAnim->scales[idx].time);

						DirectX::XMVECTOR start = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&boneAnim->scales[idx].value);
						DirectX::XMVECTOR end = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&boneAnim->scales[nextIdx].value);
						S = DirectX::XMVectorLerp(start, end, factor);
					}
				}

				localTransform = DirectX::XMMatrixAffineTransformation(S, DirectX::XMVectorZero(), R, T);
			}
		}

		DirectX::XMMATRIX globalTransform = localTransform * parentTransform;

		int boneIndex = static_cast<int>(bone - &m_skeleton.bones[0]);
		m_finalBoneMatrices[boneIndex] = bone->inverseBindMatrix * globalTransform;

		for (int childIndex : bone->children) {
			calculateBoneTransform(&m_skeleton.bones[childIndex], globalTransform);
		}
	}

}
