using System;

namespace AxionScriptCore {

	public class Bullet : Entity {
		private float m_LifeTime = 0.0f;

		public override void OnCreate() {
			Console.WriteLine($"[C#] Bullet {ID} initialized and ready!");

			RigidBody.AddForce(new Vector3(0, 0, 1), ForceMode.Impulse);
		}

		public override void OnUpdate(float timestep) {
			m_LifeTime += Time.DeltaTime;

			if (m_LifeTime > 2.0f) {
				Console.WriteLine($"[C#] Bullet {ID} destroyed!");
				Destroy();
			}
		}

		public override void OnCollisionEnter(Collision collision) {
			Console.WriteLine($"[C#] Bullet Hit {collision.OtherEntity.ID}!");

			Enemy? hitEnemy = collision.OtherEntity.As<Enemy>();

			if (hitEnemy != null) {
				hitEnemy.TakeDamage(25.0f);
			}

			//Destroy();
		}

	}

}
