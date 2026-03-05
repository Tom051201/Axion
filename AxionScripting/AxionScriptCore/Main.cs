using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	[StructLayout(LayoutKind.Sequential)]
	public unsafe struct EngineAPI {
		public delegate* unmanaged<int, byte> IsKeyDown;
	}

	public class CoreAPI {

		internal static unsafe EngineAPI API;

		[UnmanagedCallersOnly(EntryPoint = "Initialize")]
		public static unsafe void Initialize(EngineAPI* api) {
			API = *api;

			Console.WriteLine("[C#] Axion Script Engine API linked successfully!");

			bool spacePressed = Input.IsKeyDown(32);
			Console.WriteLine($"[C#] Clean Input API test. Space pressed? {spacePressed}");
		}

	}

	// -- GAME DEV API --

	public class Input {

		public static unsafe bool IsKeyDown(int keyCode) {
			byte isDown = CoreAPI.API.IsKeyDown(keyCode);
			return isDown == 1;
		}

	}



	public class ScriptManager {

		[UnmanagedCallersOnly(EntryPoint = "CreateEntityScript")]
		public static IntPtr CreateEntityScript(ulong uuidHigh, ulong uuidLow, IntPtr scriptNamePtr) {

			string? scriptName = Marshal.PtrToStringAnsi(scriptNamePtr);

			if (string.IsNullOrEmpty(scriptName)) {
				Console.WriteLine($"[C#] ERROR: C++ passed an empty script name!");
				return IntPtr.Zero;
			}

			Type? scriptType = Type.GetType(scriptName);
			if (scriptType == null) {
				Console.WriteLine($"[C#] ERROR: Could not find script class '{scriptName}'!");
				return IntPtr.Zero;
			}

			Entity? newScript = Activator.CreateInstance(scriptType) as Entity;
			if (newScript == null) {
				Console.WriteLine($"[C#] ERROR: Failed to create '{scriptName}', or it does not inherit from Entity!");
				return IntPtr.Zero;
			}

			newScript.ID = new UUID { High = uuidHigh, Low = uuidLow };
			newScript.OnCreate();

			GCHandle handle = GCHandle.Alloc(newScript, GCHandleType.Normal);

			return GCHandle.ToIntPtr(handle);
		}

		[UnmanagedCallersOnly(EntryPoint = "UpdateEntityScript")]
		public static void UpdateEntityScript(IntPtr gcHandlePtr, float timestep) {

			GCHandle handle = GCHandle.FromIntPtr(gcHandlePtr);

			Entity? script = handle.Target as Entity;
			script?.OnUpdate(timestep);
		}

		[UnmanagedCallersOnly(EntryPoint = "DestroyEntityScript")]
		public static void DestroyEntityScript(IntPtr gcHandlePtr) {
			GCHandle handle = GCHandle.FromIntPtr(gcHandlePtr);

			Entity? script = handle.Target as Entity;
			script?.OnDestroy();

			handle.Free();
		}

	}

}
