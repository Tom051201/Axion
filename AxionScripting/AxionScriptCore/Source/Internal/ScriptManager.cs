using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.IO;

namespace AxionScriptCore {

	public class ScriptManager {

		private static Assembly? s_AppAssembly = null;

		static ScriptManager() {
			AssemblyLoadContext.Default.Resolving += OnAssemblyResolve;
		}

		private static Assembly? OnAssemblyResolve(AssemblyLoadContext context, AssemblyName assemblyName) {
			if (assemblyName.Name != null && assemblyName.Name.Contains("AxionScriptCore")) {
				return typeof(ScriptManager).Assembly;
			}
			return null;
		}

		[UnmanagedCallersOnly(EntryPoint = "LoadAppAssembly")]
		public static unsafe void LoadAppAssembly(IntPtr assemblyPathPtr) {
			string? path = Marshal.PtrToStringAnsi(assemblyPathPtr);
			if (string.IsNullOrEmpty(path) || !File.Exists(path)) return;

			try {
				byte[] assemblyBytes = File.ReadAllBytes(path);
				s_AppAssembly = Assembly.Load(assemblyBytes);

				Console.WriteLine($"[C#] Successfully loaded App Assembly: {path}");
			}
			catch (Exception e) {
				Console.WriteLine($"[C#] Failed to load App Assembly: {e.Message}");
			}
		}

		[UnmanagedCallersOnly(EntryPoint = "CreateEntityScript")]
		public static IntPtr CreateEntityScript(ulong uuidHigh, ulong uuidLow, IntPtr scriptNamePtr) {

			string? scriptName = Marshal.PtrToStringAnsi(scriptNamePtr);

			if (string.IsNullOrEmpty(scriptName)) {
				Console.WriteLine($"[C#] ERROR: C++ passed an empty script name!");
				return IntPtr.Zero;
			}

			Type? scriptType = s_AppAssembly?.GetType(scriptName) ?? Type.GetType(scriptName);
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
			if (s_AppAssembly == null) return;

			try {
				foreach (var type in s_AppAssembly.GetTypes()) {
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
			catch (ReflectionTypeLoadException rtle) {
				Console.WriteLine($"[C#] Reflection Error: {rtle.Message}");
				foreach (var ex in rtle.LoaderExceptions) {
					Console.WriteLine($"  - {ex?.Message}");
				}
			}
			catch (Exception e) {
				Console.WriteLine($"[C#] Metadata Error: {e.Message}");
			}
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
