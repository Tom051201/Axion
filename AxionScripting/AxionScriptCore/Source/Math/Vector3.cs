using System;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3 {
		public float X;
		public float Y;
		public float Z;

		public Vector3(float x, float y, float z) {
			X = x;
			Y = y;
			Z = z;
		}

		public static Vector3 operator +(Vector3 v1, Vector3 v2) => new Vector3(v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z);
		public static Vector3 operator -(Vector3 v1, Vector3 v2) => new Vector3(v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z);
		public static Vector3 operator *(Vector3 v, float scalar) => new Vector3(v.X * scalar, v.Y * scalar, v.Z * scalar);
		public static Vector3 operator /(Vector3 v, float scalar) => new Vector3(v.X / scalar, v.Y / scalar, v.Z / scalar);

		public float Length() {
			return (float)Math.Sqrt(X * X + Y * Y + Z * Z);
		}

		public Vector3 Normalized() {
			float len = Length();
			if (len > 0.00001f) {
				return this / len;
			}
			return new Vector3(0, 0, 0);
		}

		public override string ToString() {
			return $"[{X}, {Y}, {Z}]";
		}

	}

}
