using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	public enum ForceMode : int {
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration
	}

	[StructLayout(LayoutKind.Sequential)]
	public unsafe struct EngineAPI {
		public delegate* unmanaged<ushort, byte> Input_IsKeyPressed;
		public delegate* unmanaged<ushort, byte> Input_IsMouseButtonPressed;
		public delegate* unmanaged<float*, float*, void> Input_GetMousePosition;

		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_GetPosition;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_SetPosition;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_GetRotation;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_SetRotation;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_GetScale;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_SetScale;

		public delegate* unmanaged<ulong, ulong, Vector3*, int, void> RigidBody_AddForce;
		public delegate* unmanaged<ulong, ulong, Vector3*, int, void> RigidBody_AddTorque;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> RigidBody_GetLinearVelocity;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> RigidBody_SetLinearVelocity;
		public delegate* unmanaged<ulong, ulong, float> RigidBody_GetMass;
		public delegate* unmanaged<ulong, ulong, float, void> RigidBody_SetMass;
	}

	public class CoreAPI {

		internal static unsafe EngineAPI API;

		[UnmanagedCallersOnly(EntryPoint = "Initialize")]
		public static unsafe void Initialize(EngineAPI* api) {
			API = *api;

			Console.WriteLine("[C#] Axion Script Engine API linked successfully!");

		}

	}

	// -- GAME DEV API --

	public class Input {

		public static unsafe bool IsKeyPressed(KeyCode keyCode) {
			byte isDown = CoreAPI.API.Input_IsKeyPressed((ushort)keyCode);
			return isDown == 1;
		}

		public static unsafe bool IsMouseButtonPressed(MouseButton button) {
			byte isDown = CoreAPI.API.Input_IsMouseButtonPressed((ushort)button);
			return isDown == 1;
		}

		public static unsafe void GetMousePosition(out float x, out float y) {
			float outX, outY;
			CoreAPI.API.Input_GetMousePosition(&outX, &outY);
			x = outX;
			y = outY;
		}

		public static float GetMouseX() {
			GetMousePosition(out float x, out float _);
			return x;
		}

		public static float GetMouseY() {
			GetMousePosition(out float _, out float y);
			return y;
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

		[UnmanagedCallersOnly(EntryPoint = "OnCollisionEnterScript")]
		public static unsafe void OnCollisionEnterScript(IntPtr gcHandlePtr, InteropCollision* collisionData) {
			GCHandle handle = GCHandle.FromIntPtr(gcHandlePtr);
			if (handle.Target is Entity script) {

				Collision col = new Collision();
				col.OtherEntity = new Entity() { ID = new UUID { High = collisionData->EntityIDHigh, Low = collisionData->EntityIDLow } };
				col.ContactPoint = collisionData->ContactPoint;
				col.ContactNormal = collisionData->ContactNormal;
				col.Impulse = collisionData->Impulse;

				script.OnCollisionEnter(col);
			}
		}

		[UnmanagedCallersOnly(EntryPoint = "OnCollisionExitScript")]
		public static unsafe void OnCollisionExitScript(IntPtr gcHandlePtr, InteropCollision* collisionData) {
			GCHandle handle = GCHandle.FromIntPtr(gcHandlePtr);
			if (handle.Target is Entity script) {

				Collision col = new Collision();
				col.OtherEntity = new Entity() { ID = new UUID { High = collisionData->EntityIDHigh, Low = collisionData->EntityIDLow } };
				col.ContactPoint = collisionData->ContactPoint;
				col.ContactNormal = collisionData->ContactNormal;
				col.Impulse = collisionData->Impulse;

				script.OnCollisionExit(col);
			}
		}

	}

}
