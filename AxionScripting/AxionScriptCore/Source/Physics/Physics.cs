using System;

namespace AxionScriptCore {

	public struct RaycastHit {
		public Entity Entity;
		public Vector3 Position;
		public Vector3 Normal;
		public float Distance;
	}

	public class Physics {

		public static unsafe bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hitInfo) {
			hitInfo = new RaycastHit();

			ulong idHi = 0, idLo = 0;
			Vector3 hitPos, hitNormal;
			float hitDistance = 0;

			byte hit = CoreAPI.API.Physics_Raycast(&origin, &direction, maxDistance, &idHi, &idLo, &hitPos, &hitNormal, &hitDistance);

			if (hit == 1) {
				hitInfo.Position = hitPos;
				hitInfo.Normal = hitNormal;
				hitInfo.Distance = hitDistance;

				if (idHi != 0 || idLo != 0) {
					hitInfo.Entity = new Entity() { ID = new UUID { High = idHi, Low = idLo } };
				}

				return true;
			}

			return false;
		}

	}

}
