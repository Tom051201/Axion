#pragma once

#include "AxionEngine/Source/scene/ScriptableEntity.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/input/Input.h"

namespace Axion {

	class CameraController : public ScriptableEntity {
	public:

		void onCreate() override {
			auto& transform = getComponent<TransformComponent>();
		}

		void onDestroy() override {

		}

		void onUpdate(Timestep ts) override {
			auto& transform = getComponent<TransformComponent>();
			auto& audio = getComponent<AudioComponent>();
			float speed = 5.0 * ts;

			if (Input::isKeyPressed(KeyCode::W)) {
				transform.position.y += speed;
			}

			if (Input::isKeyPressed(KeyCode::S)) {
				transform.position.y -= speed;
			}

			if (Input::isKeyPressed(KeyCode::G)) {
				audio.audio->play();
			}
		}

	};

}
