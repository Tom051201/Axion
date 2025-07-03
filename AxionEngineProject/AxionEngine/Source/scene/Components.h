 #pragma once

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/render/Camera.h"

namespace Axion {

	struct TagComponent {
		std::string tag = "Unnamed Entity";

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : tag(tag) {}
	};



	struct TransformComponent {
		Mat4 transform = Mat4::identity();

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Mat4& transform) : transform(transform) {}

		operator Mat4& () { return transform; }
		operator const Mat4& () const { return transform; }
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
