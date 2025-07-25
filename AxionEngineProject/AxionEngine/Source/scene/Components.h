 #pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Camera.h"
#include "AxionEngine/Source/render/Mesh.h"
#include "AxionEngine/Source/render/Material.h"

namespace Axion {

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
		Ref<Mesh> mesh;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(const Ref<Mesh>& mesh) : mesh(mesh) {}
	};



	struct MaterialComponent {
		Ref<Material> material;

		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent&) = default;
		MaterialComponent(const Ref<Material>& mat) : material(mat) {}

		const std::string& getName() const { return material->getName(); }
		const Vec4& getColor() const { return material->getColor(); }
	};



	struct DirectionalLightComponent {
		Vec3 direction = { 0.0f, 1.0f, 0.0f };
		Vec4 color = Vec4::one();

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
		DirectionalLightComponent(const Vec3& dir, const Vec4& col) : direction(dir), color(col) {}
	};



	struct SpriteRendererComponent {
		Vec4 color = Vec4::one();

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const Vec4& color) : color(color) {}
	};



	struct CameraComponent {
		Camera camera;
		bool isPrimary = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const Mat4& projection) : camera(projection) {}
	};

}
