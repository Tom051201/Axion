using System;

namespace AxionScriptCore {

	public class Player : Entity {

		public override void OnCreate() {
			Console.WriteLine($"[C#] Player script created for Entity {ID}!"); ;
		}

		public override void OnUpdate(float timestep) {
			if (Input.IsKeyDown(32)) {
				Console.WriteLine($"[C#] Entity {ID} is jumping!");
			}
		}

	}

}
