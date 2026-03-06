
namespace AxionScriptCore {

	public class RigidBody {
		private Entity m_Entity;

		internal RigidBody(Entity entity) {
			m_Entity = entity;
		}

		public unsafe void AddForce(Vector3 force, ForceMode mode = ForceMode.Force) {
			CoreAPI.API.RigidBody_AddForce(m_Entity.ID.High, m_Entity.ID.Low, &force, (int)mode);
		}

		public unsafe void AddTorque(Vector3 torque, ForceMode mode = ForceMode.Force) {
			CoreAPI.API.RigidBody_AddTorque(m_Entity.ID.High, m_Entity.ID.Low, &torque, (int)mode);
		}

		public unsafe Vector3 LinearVelocity {
			get {
				Vector3 result;
				CoreAPI.API.RigidBody_GetLinearVelocity(m_Entity.ID.High, m_Entity.ID.Low, &result);
				return result;
			}
			set {
				CoreAPI.API.RigidBody_SetLinearVelocity(m_Entity.ID.High, m_Entity.ID.Low, &value);
			}
		}

		public unsafe float Mass {
			get => CoreAPI.API.RigidBody_GetMass(m_Entity.ID.High, m_Entity.ID.Low);
			set => CoreAPI.API.RigidBody_SetMass(m_Entity.ID.High, m_Entity.ID.Low, value);
		}

	}

}
