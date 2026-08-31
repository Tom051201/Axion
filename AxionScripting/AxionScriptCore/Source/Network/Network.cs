using System;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	public static class Network {

		public static unsafe bool IsLocalPlayer(Entity entity) {
			return CoreAPI.API.Network_IsLocalPlayer(entity.ID.High, entity.ID.Low) != 0;
		}

		public static unsafe void SendEvent(Entity entity, uint eventID, byte[] payload = null) {
			ushort size = (ushort)(payload != null ? Math.Min(payload.Length, 128) : 0);

			fixed (byte* ptr = payload) {
				CoreAPI.API.Network_SendEvent(entity.ID.High, entity.ID.Low, eventID, ptr, size);
			}

		}

	}

}
