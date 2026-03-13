using System;

namespace AxionScriptCore {

	public class Enemy : Entity {
		public float Health = 100.0f;

		private Random m_Random = new Random();

		public void TakeDamage(float amount) {
			Health -= amount;

			if (Health <= 0.0f) {
				Health = 100.0f;

				float randomX = (float)(m_Random.NextDouble() * 28.0) - 14.0f;
				float fixedY = 2.5f;
				float randomZ = (float)(m_Random.NextDouble() * 28.0) - 14.0f;

				Transform.Position = new Vector3(randomX, fixedY, randomZ);

				RigidBody.LinearVelocity = new Vector3(0.0f, 0.0f, 0.0f);
				RigidBody.AngularVelocity = new Vector3(0.0f, 0.0f, 0.0f);

				Audio.Play();
			}
		}

	}

}
