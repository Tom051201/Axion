using AxionScriptCore;
using System;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	public class CoreAPI {

		internal static unsafe EngineAPI API;

		[UnmanagedCallersOnly(EntryPoint = "Initialize")]
		public static unsafe void Initialize(EngineAPI* api) {
			API = *api;

			Console.WriteLine("[C#] Axion Script Engine API linked successfully!");

		}

	}

}
