using System;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	[StructLayout(LayoutKind.Sequential)]
	public struct UUID {

		public ulong High;
		public ulong Low;

		public override string ToString() {
			return $"{High:X16}{Low:X16}";
		}

	}

}
