#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Material.h"
#include "AxionEngine/Source/render/Texture.h"
#include "AxionEngine/Source/audio/AudioClip.h"
#include "AxionEngine/Source/audio/AudioSource.h"
#include "AxionEngine/Source/scene/ScriptableEntity.h"
#include "AxionEngine/Source/physics/PhysicsMaterial.h"

namespace Axion {

	struct UUIDComponent {
		UUID id;

		UUIDComponent() = default;
		UUIDComponent(const UUIDComponent&) = default;
		UUIDComponent(UUID id) : id(id) {}
		UUIDComponent(uint64_t high, uint64_t low) : id(high, low) {}
	};



	struct TagComponent {
		std::string tag = "Unnamed Entity";

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : tag(tag) {}
	};



	struct TransformComponent {
		Vec3 position = Vec3(0.0f, 0.0f, 0.0f);
		Quat rotation = Quat::identity();
		Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Vec3& pos, const Quat& rot, const Vec3& sca) : position(pos), rotation(rot), scale(sca) {}

		Mat4 getTransform() const {
			return Mat4::TRS(position, rotation, scale);
		}

		Vec3 getEulerAngles() const {
			return rotation.toEulerAngles();
		}

		void setEulerAngles(const Vec3& eulerDegrees) {
			rotation = Quat::fromEulerAngles(eulerDegrees);
		}

	};



	struct MeshComponent {
		AssetHandle<Mesh> handle;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(const AssetHandle<Mesh>& handle) : handle(handle) {}
	};



	struct MaterialComponent {
		AssetHandle<Material> handle;

		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent&) = default;
		MaterialComponent(const AssetHandle<Material>& handle) : handle(handle) {}
	};



	struct DirectionalLightComponent {
		Vec4 color = Vec4::one();

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
		DirectionalLightComponent(const Vec4& col) : color(col) {}
	};



	struct PointLightComponent {
		Vec4 color = Vec4::one();
		float intensity = 1.0f;
		float radius = 10.0f;
		float falloff = 1.0f;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};



	struct SpotLightComponent {
		Vec4 color = Vec4::one();
		float intensity = 1.0f;
		float range = 10.0f;
		float innerConeAngle = 12.5f; // degrees
		float outerConeAngle = 17.5f; // degrees

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;
	};



	struct CameraComponent {
		Camera camera;
		bool isPrimary = false;
		bool fixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(float aspectRatio) : camera(aspectRatio) {}
	};



	struct AudioComponent {
		Ref<AudioSource> audio = nullptr;
		bool isListener = false;
		bool isSource = true;

		AudioComponent() = default;
		AudioComponent(const AudioComponent&) = default;
		AudioComponent(const Ref<AudioSource>& source) : audio(source), isListener(false), isSource(true) {}
		AudioComponent(bool isListener, bool isSource) : audio(), isListener(isListener), isSource(isSource) {}
	};



	struct SpriteComponent {
		AssetHandle<Texture2D> texture;
		Vec4 tint = Vec4::one();

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(const AssetHandle<Texture2D>& tex, const Vec4& tint) : texture(tex), tint(tint) {}
	};



	struct NativeScriptComponent {
		// TODO: add to scene serializer
		ScriptableEntity* instance = nullptr;

		ScriptableEntity* (*instantiateScript)();
		void (*destroyScript)(NativeScriptComponent*);

		template<typename T>
		void bind() {
			instantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			destroyScript = [](NativeScriptComponent* nsc) { delete nsc->instance; nsc->instance = nullptr; };
		}

	};



	struct RigidBodyComponent {
		enum class BodyType { Static, Dynamic };
		BodyType type = BodyType::Dynamic;

		float mass = 1.0f;
		bool isKinematic = false;

		float linearDamping = 0.0f;
		float angularDamping = 0.05f;

		bool fixedRotationX = false;
		bool fixedRotationY = false;
		bool fixedRotationZ = false;

		bool enableCCD = false;
		bool useGlobalGravity = true;

		void* runtimeActor = nullptr;

		RigidBodyComponent() = default;
		RigidBodyComponent(const RigidBodyComponent&) = default;
	};



	struct BoxColliderComponent {
		Vec3 halfExtents = { 0.5f, 0.5f, 0.5f };
		Vec3 offset = { 0.0f, 0.0f, 0.0f };

		AssetHandle<PhysicsMaterial> material;

		void* runtimeShape = nullptr;

		BoxColliderComponent() = default;
		BoxColliderComponent(const BoxColliderComponent&) = default;
	};



	struct SphereColliderComponent {
		float radius = 0.5f;
		Vec3 offset = { 0.0f, 0.0f, 0.0f };

		AssetHandle<PhysicsMaterial> material;

		void* runtimeShape = nullptr;

		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent&) = default;
	};



	struct CapsuleColliderComponent {
		float radius = 0.5f;
		float halfHeight = 1.0f;
		Vec3 offset = { 0.0f, 0.0f, 0.0f };

		AssetHandle<PhysicsMaterial> material;

		void* runtimeShape = nullptr;

		CapsuleColliderComponent() = default;
		CapsuleColliderComponent(const CapsuleColliderComponent&) = default;
	};



	struct GravitySourceComponent {
		enum class Type { Directional, Point };
		Type type = Type::Point;

		float strength = 9.81f; // Acceleration magnitude
		float radius = 100.0f;
		bool affectKinematic = false;

		GravitySourceComponent() = default;
		GravitySourceComponent(const GravitySourceComponent&) = default;
	};

}
