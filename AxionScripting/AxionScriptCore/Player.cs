using System;

namespace AxionScriptCore {

	public class Player : Entity {

		public float MoveSpeed = 5.0f;
		public float TurnSpeed = 3.0f;

		//public float FireRate = 2.0f;
		//private float m_FireTimer = 0.0f;

		public override void OnCreate() {

		}

		public override void OnUpdate(float timestep) {

			//m_FireTimer += timestep;

			//if (Input.IsMouseButtonPressed(MouseButton.Right) && m_FireTimer >= FireRate) {
			//	m_FireTimer = 0.0f;
			//
			//	Entity bullet = Entity.InstantiatePrefab("prefabs/Bullet.axprefab");
			//
			//	if (bullet != null) {
			//		bullet.Transform.Rotation = Transform.Rotation;
			//		bullet.Transform.Position = Transform.Position + (Transform.Forward * 1.5f);
			//		Audio.Play();
			//	}
			//
			//}

			Vector3 currentVel = RigidBody.LinearVelocity;
			Vector3 targetVelocity = new Vector3(0.0f, currentVel.Y, 0.0f);

			if (Input.IsKeyPressed(KeyCode.W)) targetVelocity += Transform.Forward * MoveSpeed;
			if (Input.IsKeyPressed(KeyCode.S)) targetVelocity -= Transform.Forward * MoveSpeed;
			if (Input.IsKeyPressed(KeyCode.A)) targetVelocity -= Transform.Right * MoveSpeed;
			if (Input.IsKeyPressed(KeyCode.D)) targetVelocity += Transform.Right * MoveSpeed;

			RigidBody.LinearVelocity = targetVelocity;

			float turnInput = 0.0f;

			if (Input.IsKeyPressed(KeyCode.Q)) {
				turnInput = -TurnSpeed;
			}
			if (Input.IsKeyPressed(KeyCode.E)) {
				turnInput = TurnSpeed;
			}

			RigidBody.AngularVelocity = new Vector3(0.0f, turnInput, 0.0f);

			if (Input.IsKeyPressed(KeyCode.Space) && currentVel.Y <= 0.1f) {
				RigidBody.AddForce(new Vector3(0, 1.0f, 0), ForceMode.Impulse);
			}

		}

		public override void OnCollisionEnter(Collision collision) {

		}

	}

}
