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

		public static unsafe bool SweepBox(Vector3 origin, Vector3 halfExtents, Vector3 orientation, Vector3 direction, float maxDistance, out RaycastHit hitInfo) {
			hitInfo = new RaycastHit();
			ulong idHi = 0, idLo = 0;
			Vector3 hitPos, hitNormal; float hitDistance = 0;

			byte hit = CoreAPI.API.Physics_SweepBox(&origin, &halfExtents, &orientation, &direction, maxDistance, &idHi, &idLo, &hitPos, &hitNormal, &hitDistance);

			if (hit == 1) {
				hitInfo.Position = hitPos; hitInfo.Normal = hitNormal; hitInfo.Distance = hitDistance;
				if (idHi != 0 || idLo != 0) hitInfo.Entity = new Entity() { ID = new UUID { High = idHi, Low = idLo } };
				return true;
			}
			return false;
		}

		public static unsafe bool SweepSphere(Vector3 origin, float radius, Vector3 direction, float maxDistance, out RaycastHit hitInfo) {
			hitInfo = new RaycastHit();

			ulong idHi = 0, idLo = 0;
			Vector3 hitPos, hitNormal;
			float hitDistance = 0;

			byte hit = CoreAPI.API.Physics_SweepSphere(&origin, radius, &direction, maxDistance, &idHi, &idLo, &hitPos, &hitNormal, &hitDistance);

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

		public static unsafe bool SweepCapsule(Vector3 origin, float radius, float halfHeight, Vector3 orientation, Vector3 direction, float maxDistance, out RaycastHit hitInfo) {
			hitInfo = new RaycastHit();
			ulong idHi = 0, idLo = 0;
			Vector3 hitPos, hitNormal; float hitDistance = 0;

			byte hit = CoreAPI.API.Physics_SweepCapsule(&origin, radius, halfHeight, &orientation, &direction, maxDistance, &idHi, &idLo, &hitPos, &hitNormal, &hitDistance);

			if (hit == 1) {
				hitInfo.Position = hitPos; hitInfo.Normal = hitNormal; hitInfo.Distance = hitDistance;
				if (idHi != 0 || idLo != 0) hitInfo.Entity = new Entity() { ID = new UUID { High = idHi, Low = idLo } };
				return true;
			}
			return false;
		}

		public static unsafe Entity[] OverlapBox(Vector3 center, Vector3 halfExtents, Vector3 orientation) {
			ulong[] idsHi = new ulong[64]; ulong[] idsLo = new ulong[64];
			int hitCount = 0;

			fixed (ulong* ptrHi = idsHi) fixed (ulong* ptrLo = idsLo) {
				hitCount = CoreAPI.API.Physics_OverlapBox(&center, &halfExtents, &orientation, ptrHi, ptrLo, 64);
			}

			Entity[] hitEntities = new Entity[hitCount];
			for (int i = 0; i < hitCount; i++) {
				hitEntities[i] = new Entity() { ID = new UUID { High = idsHi[i], Low = idsLo[i] } };
			}
			return hitEntities;
		}

		public static unsafe Entity[] OverlapSphere(Vector3 center, float radius) {
			ulong[] idsHi = new ulong[64];
			ulong[] idsLo = new ulong[64];
			int hitCount = 0;

			fixed (ulong* ptrHi = idsHi)
			fixed (ulong* ptrLo = idsLo) {
				hitCount = CoreAPI.API.Physics_OverlapSphere(&center, radius, ptrHi, ptrLo, 64);
			}

			Entity[] hitEntities = new Entity[hitCount];
			for (int i = 0; i < hitCount; i++) {
				hitEntities[i] = new Entity() {
					ID = new UUID { High = idsHi[i], Low = idsLo[i] }
				};
			}

			return hitEntities;
		}

		public static unsafe Entity[] OverlapCapsule(Vector3 center, float radius, float halfHeight, Vector3 orientation) {
			ulong[] idsHi = new ulong[64]; ulong[] idsLo = new ulong[64];
			int hitCount = 0;

			fixed (ulong* ptrHi = idsHi) fixed (ulong* ptrLo = idsLo) {
				hitCount = CoreAPI.API.Physics_OverlapCapsule(&center, radius, halfHeight, &orientation, ptrHi, ptrLo, 64);
			}

			Entity[] hitEntities = new Entity[hitCount];
			for (int i = 0; i < hitCount; i++) {
				hitEntities[i] = new Entity() { ID = new UUID { High = idsHi[i], Low = idsLo[i] } };
			}
			return hitEntities;
		}

	}

}
