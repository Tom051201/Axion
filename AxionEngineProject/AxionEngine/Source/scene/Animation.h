#pragma once

#include "axpch.h"

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/Timestep.h"
#include "AxionEngine/Source/render/SkeletalMesh.h"

namespace Axion {

	template<typename T>
	struct Keyframe {
		float time;
		T value;
	};

	struct BoneAnimation {
		std::string boneName;
		std::vector<Keyframe<Vec3>> positions;
		std::vector<Keyframe<Vec4>> rotations;
		std::vector<Keyframe<Vec3>> scales;
	};

	class AnimationClip {
	public:

		float duration;
		float ticksPerSecond;
		std::vector<BoneAnimation> boneAnimations;

		const BoneAnimation* getBoneAnimation(const std::string& name) const;
	};

	class Animator {
	public:

		Animator(const Skeleton& skeleton);

		void playAnimation(const Ref<AnimationClip>& clip);
		void update(Timestep ts);
		const std::vector<DirectX::XMMATRIX>& getFinalMatrices() const;

	private:

		void calculateBoneTransform(const Bone* bone, DirectX::XMMATRIX parentTransform);

		const Skeleton& m_skeleton;
		Ref<AnimationClip> m_currentClip;
		float m_currentTime = 0.0f;
		std::vector<DirectX::XMMATRIX> m_finalBoneMatrices;

	};


}
