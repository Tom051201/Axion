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

		// -- INPUT --
		public delegate* unmanaged<ushort, byte> Input_IsKeyPressed;
		public delegate* unmanaged<ushort, byte> Input_IsMouseButtonPressed;
		public delegate* unmanaged<float*, float*, void> Input_GetMousePosition;

		// -- TRANSFORM --
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_GetPosition;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_SetPosition;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_GetRotation;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_SetRotation;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_GetScale;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_SetScale;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_GetForward;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_GetRight;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> Transform_GetUp;

		// -- PHYSICS --
		public delegate* unmanaged<ulong, ulong, Vector3*, int, void> RigidBody_AddForce;
		public delegate* unmanaged<ulong, ulong, Vector3*, int, void> RigidBody_AddTorque;
		public delegate* unmanaged<ulong, ulong, Vector3*, float, float, void> RigidBody_AddRadialImpulse;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> RigidBody_GetLinearVelocity;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> RigidBody_SetLinearVelocity;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> RigidBody_GetAngularVelocity;
		public delegate* unmanaged<ulong, ulong, Vector3*, void> RigidBody_SetAngularVelocity;
		public delegate* unmanaged<ulong, ulong, float> RigidBody_GetMass;
		public delegate* unmanaged<ulong, ulong, float, void> RigidBody_SetMass;
		public delegate* unmanaged<Vector3*, Vector3*, float, ulong*, ulong*, Vector3*, Vector3*, float*, byte> Physics_Raycast;

		// -- AUDIO --
		public delegate* unmanaged<ulong, ulong, void> Audio_Play;
		public delegate* unmanaged<ulong, ulong, void> Audio_Stop;
		public delegate* unmanaged<ulong, ulong, float, void> Audio_SetVolume;
		public delegate* unmanaged<ulong, ulong, float> Audio_GetVolume;
		public delegate* unmanaged<ulong, ulong, float, void> Audio_SetPitch;
		public delegate* unmanaged<ulong, ulong, float> Audio_GetPitch;

		// -- ENTITY --
		public delegate* unmanaged<IntPtr, ulong*, ulong*, void> Entity_Instantiate;
		public delegate* unmanaged<IntPtr, ulong*, ulong*, void> Entity_InstantiatePrefab;
		public delegate* unmanaged<ulong, ulong, void> Entity_Destroy;
		public delegate* unmanaged<ulong, ulong, int, void> Entity_AddComponent;
		public delegate* unmanaged<ulong, ulong, IntPtr, void> Entity_AddScript;
		public delegate* unmanaged<IntPtr, ulong*, ulong*, void> Entity_FindEntityByName;
		public delegate* unmanaged<ulong, ulong, IntPtr> Entity_GetScriptInstance;
		public delegate* unmanaged<ulong, ulong, int, void> Entity_EmitParticles;

		// -- ANIMATION --
		public delegate* unmanaged<ulong, ulong, void> Animation_Play;
		public delegate* unmanaged<ulong, ulong, void> Animation_Stop;
		public delegate* unmanaged<ulong, ulong, byte> Animation_IsPlaying;

		// -- REFLECTION --
		public delegate* unmanaged<IntPtr, IntPtr, int, void> Script_RegisterField;

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

		[UnmanagedCallersOnly(EntryPoint = "UpdateDeltaTime")]
		public static void UpdateDeltaTime(float deltaTime) {
			Time.DeltaTime = deltaTime;
		}

		[UnmanagedCallersOnly(EntryPoint = "GenerateScriptMetadata")]
		public static unsafe void GenerateScriptMetadata() {
			var assembly = typeof(ScriptManager).Assembly;

			foreach (var type in assembly.GetTypes()) {
				if (type.IsSubclassOf(typeof(Entity))) {
					IntPtr classNamePtr = Marshal.StringToHGlobalAnsi(type.FullName);

					foreach (var field in type.GetFields(System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.Instance)) {
						int fieldType = -1;
						if (field.FieldType == typeof(float)) fieldType = 0; // Float
						else if (field.FieldType == typeof(Vector3)) fieldType = 1; // Vector3

						if (fieldType != -1) {
							IntPtr fieldNamePtr = Marshal.StringToHGlobalAnsi(field.Name);
							CoreAPI.API.Script_RegisterField(classNamePtr, fieldNamePtr, fieldType);
							Marshal.FreeHGlobal(fieldNamePtr);
						}
					}
					Marshal.FreeHGlobal(classNamePtr);
				}
			}
			Console.WriteLine("[C#] Reflection Metadata Generated!");
		}

		[UnmanagedCallersOnly(EntryPoint = "GetFieldValue_Float")]
		public static float GetFieldValue_Float(IntPtr gcHandlePtr, IntPtr fieldNamePtr) {
			GCHandle handle = GCHandle.FromIntPtr(gcHandlePtr);
			if (handle.Target is Entity script) {
				string? fieldName = Marshal.PtrToStringAnsi(fieldNamePtr);
				if (string.IsNullOrEmpty(fieldName)) return 0.0f;
				var field = script.GetType().GetField(fieldName);
				if (field != null) {
					object? value = field.GetValue(script);
					if (value is float f) return f;
				}
			}
			return 0.0f;
		}

		[UnmanagedCallersOnly(EntryPoint = "SetFieldValue_Float")]
		public static void SetFieldValue_Float(IntPtr gcHandlePtr, IntPtr fieldNamePtr, float value) {
			GCHandle handle = GCHandle.FromIntPtr(gcHandlePtr);
			if (handle.Target is Entity script) {
				string? fieldName = Marshal.PtrToStringAnsi(fieldNamePtr);
				if (string.IsNullOrEmpty(fieldName)) return;
				var field = script.GetType().GetField(fieldName);
				field?.SetValue(script, value);
			}
		}

		[UnmanagedCallersOnly(EntryPoint = "GetFieldValue_Vector3")]
		public static unsafe void GetFieldValue_Vector3(IntPtr gcHandlePtr, IntPtr fieldNamePtr, float* outVal) {
			GCHandle handle = GCHandle.FromIntPtr(gcHandlePtr);
			if (handle.Target is Entity script) {
				string? fieldName = Marshal.PtrToStringAnsi(fieldNamePtr);
				if (!string.IsNullOrEmpty(fieldName)) {
					var field = script.GetType().GetField(fieldName);
					if (field != null) {
						object? value = field.GetValue(script);
						if (value is Vector3 val) {
							outVal[0] = val.X; outVal[1] = val.Y; outVal[2] = val.Z;
						}
					}
				}
			}
		}

		[UnmanagedCallersOnly(EntryPoint = "SetFieldValue_Vector3")]
		public static unsafe void SetFieldValue_Vector3(IntPtr gcHandlePtr, IntPtr fieldNamePtr, float* inVal) {
			GCHandle handle = GCHandle.FromIntPtr(gcHandlePtr);
			if (handle.Target is Entity script) {
				string? fieldName = Marshal.PtrToStringAnsi(fieldNamePtr);
				if (!string.IsNullOrEmpty(fieldName)) {
					var field = script.GetType().GetField(fieldName);
					field?.SetValue(script, new Vector3(inVal[0], inVal[1], inVal[2]));
				}
			}
		}


	}

}
