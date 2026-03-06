using System;

namespace AxionScriptCore {

	public class Player : Entity {

		private float m_FireTimer = 0.0f;

		public override void OnCreate() {
			Console.WriteLine($"[C#] Player script created for Entity {ID}!"); ;
		}

		public override void OnUpdate(float timestep) {
			m_FireTimer += Time.DeltaTime;

			// Jumping
			if (Input.IsKeyPressed(KeyCode.Space)) {
				RigidBody.AddForce(new Vector3(0, 1.0f, 0), ForceMode.Impulse);
			}

			// Shooting
			if (Input.IsMouseButtonPressed(MouseButton.Left) && m_FireTimer >= 0.5f) {
				m_FireTimer = 0.0f;

				Entity bullet = Entity.InstantiatePrefab("prefabs/Bullet.axprefab");
				if (bullet != null) {
					bullet.Transform.Position = new Vector3(Transform.Position.X, Transform.Position.Y, Transform.Position.Z + 2.0f);
				}

				Vector3 forward = new Vector3(0, 0, 1);

				if (Physics.Raycast(Transform.Position, forward, 1000.0f, out RaycastHit hit)) {
					Console.WriteLine($"[C#] Raycast Hit! Entity: {hit.Entity?.ID}");
					Console.WriteLine($"[C#] Hit Distance: {hit.Distance}");
					Console.WriteLine($"[C#] Hit Position: {hit.Position.X}, {hit.Position.Y}, {hit.Position.Z}");

					// Optional: If you shot a rigidbody, launch it into the air!
					hit.Entity?.RigidBody?.AddForce(new Vector3(0, 10.0f, 0), ForceMode.Impulse);
				}
			}

		}

		public override void OnCollisionEnter(Collision collision) {
			float impactForce = collision.Impulse.Y;

			if (impactForce > 0.5f) {
				float calculatedVol = Math.Clamp(impactForce / 50.0f, 0.1f, 1.0f);

				Random rand = new Random();
				Audio.Pitch = 0.8f + (float)rand.NextDouble() * 0.4f;

				Audio.Volume = calculatedVol;
				Audio.Play();

				Console.WriteLine($"[C#] THUD! Volume: {calculatedVol}");
			}

		}

	}

}
