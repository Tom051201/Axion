using System;

namespace AxionScriptCore {

	public class Entity {

		public UUID ID { get; internal set; }

		public Transform Transform { get; private set; }
		public RigidBody RigidBody { get; private set; }

		public Entity() {
			Transform = new Transform(this);
			RigidBody = new RigidBody(this);
		}

		public virtual void OnCreate() {}
		public virtual void OnDestroy() {}
		public virtual void OnUpdate(float timestep) {}

		public virtual void OnCollisionEnter(Collision collision) {}
		public virtual void OnCollisionExit(Collision collision) {}

	}

}
