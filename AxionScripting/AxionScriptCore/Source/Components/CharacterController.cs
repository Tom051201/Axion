using System;

namespace AxionScriptCore {

	public class CharacterController {
		private Entity m_Entity;

		internal CharacterController(Entity entity) {
			m_Entity = entity;
		}

		public unsafe void Move(Vector3 displacement, float timestep) {
			CoreAPI.API.CCT_Move(m_Entity.ID.High, m_Entity.ID.Low, &displacement, timestep);
		}

		public unsafe bool IsGrounded {
			get {
				return CoreAPI.API.CCT_IsGrounded(m_Entity.ID.High, m_Entity.ID.Low) == 1;
			}
		}

	}

}
