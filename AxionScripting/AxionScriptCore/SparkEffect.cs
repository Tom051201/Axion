using System;

namespace AxionScriptCore {

	public class SparkEffect : Entity {
		public float m_LifeTime = 0.0f;

		public override void OnCreate() {
			EmitParticles(30);
		}

		public override void OnUpdate(float timestep) {
			m_LifeTime += Time.DeltaTime;

			if (m_LifeTime > 1.0f) {
				Destroy();
			}
		}
	}

}
