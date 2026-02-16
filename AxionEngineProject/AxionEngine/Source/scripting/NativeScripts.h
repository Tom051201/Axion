#pragma once

#include "AxionEngine/Source/scene/ScriptableEntity.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/input/Input.h"

namespace Axion {

	class CameraController : public ScriptableEntity {
	public:

		void onCreate() override {
			auto& camera = getComponent<CameraComponent>();
			camera.isPrimary = true;
			camera.camera.setPerspective(Math::toRadians(45.0f), 0.01f, 1000.0f);
		}

		void onDestroy() override {}

		void onUpdate(Timestep ts) override {
			auto& transform = getComponent<TransformComponent>();
			auto& audio = getComponent<AudioComponent>();
			float speed = 5.0f * ts;

			if (Input::isKeyPressed(KeyCode::Numpad5)) transform.position.y += speed;
			if (Input::isKeyPressed(KeyCode::Numpad2)) transform.position.y -= speed;
			if (Input::isKeyPressed(KeyCode::Numpad3)) transform.position.x -= speed;
			if (Input::isKeyPressed(KeyCode::Numpad1)) transform.position.x += speed;
			if (Input::isKeyPressed(KeyCode::Numpad6)) transform.position.z += speed;
			if (Input::isKeyPressed(KeyCode::Numpad4)) transform.position.z -= speed;

			if (Input::isKeyPressed(KeyCode::Numpad0)) {
				audio.audio->play();
			}
		}

	};

}
