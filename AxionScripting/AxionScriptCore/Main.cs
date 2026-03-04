using System;
using System.Runtime.InteropServices;

namespace AxionScriptCore {
	public class CoreAPI {
		[UnmanagedCallersOnly(EntryPoint = "InitializeScriptEngine")]
		public static void Init() {
			Console.WriteLine("[C#] Axion Script Engine (v10.0) initialized successfully!");
		}
	}
}
