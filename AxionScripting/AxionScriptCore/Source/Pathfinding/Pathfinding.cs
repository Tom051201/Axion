using System;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	public static class Pathfinding {

		public static unsafe bool BakeProfile(string profileName) {
			IntPtr namePtr = Marshal.StringToHGlobalAnsi(profileName);
			bool result = CoreAPI.API.NavMesh_BakeProfile(namePtr) == 1;
			Marshal.FreeHGlobal(namePtr);
			return result;
		}

		public static unsafe Vector3[] CalculatePath(string profileName, Vector3 start, Vector3 end) {
			const int maxPoints = 128;
			float[] buffer = new float[maxPoints * 3];
			int pointCount = 0;

			IntPtr namePtr = Marshal.StringToHGlobalAnsi(profileName);

			fixed (float* ptr = buffer) {
				pointCount = CoreAPI.API.NavMesh_CalculatePath(namePtr, &start, &end, ptr, maxPoints);
			}

			Marshal.FreeHGlobal(namePtr);

			if (pointCount <= 0) return new Vector3[0];

			Vector3[] path = new Vector3[pointCount];
			for (int i = 0; i < pointCount; i++) {
				path[i] = new Vector3(buffer[i * 3 + 0], buffer[i * 3 + 1], buffer[i * 3 + 2]);
			}
			return path;
		}

		public static unsafe Vector3 GetNearestPoint(string profileName, Vector3 point) {
			Vector3 result;
			IntPtr namePtr = Marshal.StringToHGlobalAnsi(profileName);
			CoreAPI.API.NavMesh_GetNearestPoint(namePtr, &point, &result);
			Marshal.FreeHGlobal(namePtr);
			return result;
		}

		public static unsafe Vector3 GetRandomPoint(string profileName, Vector3 center, float radius) {
			Vector3 result;
			IntPtr namePtr = Marshal.StringToHGlobalAnsi(profileName);
			CoreAPI.API.NavMesh_GetRandomPoint(namePtr, &center, radius, &result);
			Marshal.FreeHGlobal(namePtr);
			return result;
		}

		public static unsafe bool Raycast(string profileName, Vector3 start, Vector3 end, out Vector3 hitPoint) {
			Vector3 hp;
			IntPtr namePtr = Marshal.StringToHGlobalAnsi(profileName);
			bool hitWall = CoreAPI.API.NavMesh_Raycast(namePtr, &start, &end, &hp) == 1;
			Marshal.FreeHGlobal(namePtr);

			hitPoint = hp;
			return hitWall;
		}

	}

}
