
namespace AxionScriptCore {

	public class Transform {
		private Entity m_Entity;

		internal Transform(Entity entity) {
			m_Entity = entity;
		}

		public unsafe Vector3 Position {
			get {
				Vector3 result;
				CoreAPI.API.Transform_GetPosition(m_Entity.ID.High, m_Entity.ID.Low, &result);
				return result;
			}
			set {
				CoreAPI.API.Transform_SetPosition(m_Entity.ID.High, m_Entity.ID.Low, &value);
			}
		}

		public unsafe Vector3 Rotation {
			get {
				Vector3 result;
				CoreAPI.API.Transform_GetRotation(m_Entity.ID.High, m_Entity.ID.Low, &result);
				return result;
			}
			set {
				CoreAPI.API.Transform_SetRotation(m_Entity.ID.High, m_Entity.ID.Low, &value);
			}
		}

		public unsafe Vector3 Scale {
			get {
				Vector3 result;
				CoreAPI.API.Transform_GetScale(m_Entity.ID.High, m_Entity.ID.Low, &result);
				return result;
			}
			set {
				CoreAPI.API.Transform_SetScale(m_Entity.ID.High, m_Entity.ID.Low, &value);
			}
		}

		public unsafe Vector3 Forward {
			get {
				Vector3 result;
				CoreAPI.API.Transform_GetForward(m_Entity.ID.High, m_Entity.ID.Low, &result);
				return result;
			}
		}

		public unsafe Vector3 Right {
			get {
				Vector3 result;
				CoreAPI.API.Transform_GetRight(m_Entity.ID.High, m_Entity.ID.Low, &result);
				return result;
			}
		}

		public unsafe Vector3 Up {
			get {
				Vector3 result;
				CoreAPI.API.Transform_GetUp(m_Entity.ID.High, m_Entity.ID.Low, &result);
				return result;
			}
		}

	}
}
