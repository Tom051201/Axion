using System;

namespace AxionScriptCore {

	public abstract class Entity {

		public UUID ID { get; internal set; }

		public virtual void OnCreate() {}
		public virtual void OnDestroy() {}
		public virtual void OnUpdate(float timestep) {}

	}

}
