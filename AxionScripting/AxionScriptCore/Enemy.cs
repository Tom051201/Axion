using System;

namespace AxionScriptCore {

	public class Enemy : Entity {
		public float Health = 100.0f;

		public void TakeDamage(float amount) {
			Health -= amount;
			Console.WriteLine($"[C#] Enemy took {amount} damage! Health is now {Health}");

			if (Health <= 0.0f) {
				Console.WriteLine($"[C#] Enemy destroyed!");
				Destroy();
			}
		}

	}

}
