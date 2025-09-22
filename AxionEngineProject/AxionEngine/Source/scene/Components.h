#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/core/UUID.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Material.h"

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
		Vec3 rotation = Vec3(0.0f, 0.0f, 0.0f); // euler in degrees
		Vec3 scale = Vec3(1.0f, 1.0f, 1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Vec3& pos, Vec3& rot, Vec3& sca) : position(pos), rotation(rot), scale(sca) {}

		Mat4 getTransform() const {
			return Mat4::TRS(position, rotation, scale);
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

		//const std::string& getName() const { return material->getName(); }
		//const Vec4& getColor() const { return material->getColor(); }
		//Vec4& getColor() { return material->getColor(); }
	};



	struct DirectionalLightComponent {
		Vec3 direction = { 0.0f, 1.0f, 0.0f };
		Vec4 color = Vec4::one();

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
		DirectionalLightComponent(const Vec3& dir, const Vec4& col) : direction(dir), color(col) {}
	};



	struct CameraComponent {
		Camera camera;
		bool isPrimary = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const Mat4& projection) : camera(projection) {}
	};



	struct ConstantBufferComponent {
		Ref<ConstantBuffer> uploadBuffer;

		ConstantBufferComponent() = default;
		ConstantBufferComponent(const ConstantBufferComponent&) = default;
		ConstantBufferComponent(const Ref<ConstantBuffer>& uploadBuffer) : uploadBuffer(uploadBuffer) {}
	};

}
