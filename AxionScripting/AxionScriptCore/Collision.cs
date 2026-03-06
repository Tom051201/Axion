using System.Runtime.InteropServices;

namespace AxionScriptCore {

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropCollision {
		public ulong EntityIDHigh;
		public ulong EntityIDLow;
		public Vector3 ContactPoint;
		public Vector3 ContactNormal;
		public Vector3 Impulse;
	}



	public class Collision {
		public Entity OtherEntity { get; internal set; } = null!;
		public Vector3 ContactPoint { get; internal set; }
		public Vector3 ContactNormal { get; internal set; }
		public Vector3 Impulse { get; internal set; }
	}

}
