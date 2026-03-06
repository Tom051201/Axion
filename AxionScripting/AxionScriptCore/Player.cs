using System;

namespace AxionScriptCore {

	public class Player : Entity {

		public override void OnCreate() {
			Console.WriteLine($"[C#] Player script created for Entity {ID}!"); ;
		}

		public override void OnUpdate(float timestep) {
			Vector3 pos = Transform.Position;

			if (Input.IsKeyPressed(KeyCode.Space)) {
				RigidBody.AddForce(new Vector3(0, 1.0f, 0), ForceMode.Impulse);
			}

		}

		public override void OnCollisionEnter(Collision collision) {
			Console.WriteLine($"[C#] Hit {collision.OtherEntity.ID}!");
			Console.WriteLine($"[C#] Contact Normal: {collision.ContactNormal}");
			Console.WriteLine($"[C#] Hit Force (Impulse): {collision.Impulse}");
		}

	}

}
