using System;
using System.Text;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	public class NavMeshAgent {

		private Entity m_Entity;

		internal NavMeshAgent(Entity entity) {
			m_Entity = entity;
		}

		public unsafe string ProfileName {
			get {
				const int maxLength = 256;
				byte[] buffer = new byte[maxLength];

				fixed (byte* pBuffer = buffer) {
					CoreAPI.API.Agent_GetProfileName(m_Entity.ID.High, m_Entity.ID.Low, pBuffer, maxLength);

					int length = 0;
					while (length < maxLength && pBuffer[length] != 0) {
						length++;
					}
					return Encoding.UTF8.GetString(buffer, 0, length);
				}
			}
			set {
				IntPtr ptr = Marshal.StringToHGlobalAnsi(value);
				CoreAPI.API.Agent_SetProfileName(m_Entity.ID.High, m_Entity.ID.Low, ptr);
				Marshal.FreeHGlobal(ptr);
			}
		}

		public unsafe float Speed {
			get => CoreAPI.API.Agent_GetSpeed(m_Entity.ID.High, m_Entity.ID.Low);
			set => CoreAPI.API.Agent_SetSpeed(m_Entity.ID.High, m_Entity.ID.Low, value);
		}

	}
}
