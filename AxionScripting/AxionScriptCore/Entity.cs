using System;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

	public enum ComponentType {
		RigidBody = 0,
		BoxCollider = 1,
		SphereCollider = 2,
		CapuleCollider = 3,
		Audio = 4,
		ParticleSystem = 5
	}

	public class Entity {

		public UUID ID { get; internal set; }

		public Transform Transform { get; private set; }
		public RigidBody RigidBody { get; private set; }
		public AudioSource Audio { get; private set; }
		public Animator Animator { get; private set; }

		public Entity() {
			Transform = new Transform(this);
			RigidBody = new RigidBody(this);
			Audio = new AudioSource(this);
			Animator = new Animator(this);
		}

		public virtual void OnCreate() {}
		public virtual void OnDestroy() {}
		public virtual void OnUpdate(float timestep) {}

		public virtual void OnCollisionEnter(Collision collision) {}
		public virtual void OnCollisionExit(Collision collision) {}

		public static unsafe Entity Instantiate(string name = "New Entity") {
			IntPtr namePtr = Marshal.StringToHGlobalAnsi(name);

			ulong hi, lo;
			CoreAPI.API.Entity_Instantiate(namePtr, &hi, &lo);
			Marshal.FreeHGlobal(namePtr);

			return new Entity() { ID = new UUID { High = hi, Low = lo } };
		}

		public static unsafe Entity InstantiatePrefab(string filepath) {
			IntPtr pathPtr = Marshal.StringToHGlobalAnsi(filepath);

			ulong hi = 0, lo = 0;
			CoreAPI.API.Entity_InstantiatePrefab(pathPtr, &hi, &lo);
			Marshal.FreeHGlobal(pathPtr);

			if (hi == 0 && lo == 0) return null!;

			return new Entity() { ID = new UUID { High = hi, Low = lo } };
		}

		public unsafe void Destroy() {
			CoreAPI.API.Entity_Destroy(ID.High, ID.Low);
		}

		public unsafe void AddComponent(ComponentType type) {
			CoreAPI.API.Entity_AddComponent(ID.High, ID.Low, (int)type);
		}

		public unsafe void AddScript(string scriptName) {
			IntPtr namePtr = Marshal.StringToHGlobalAnsi(scriptName);
			CoreAPI.API.Entity_AddScript(ID.High, ID.Low, namePtr);
			Marshal.FreeHGlobal(namePtr);
		}

		public static unsafe Entity? FindEntityByName(string name) {
			IntPtr namePtr = Marshal.StringToHGlobalAnsi(name);

			ulong hi = 0, lo = 0;
			CoreAPI.API.Entity_FindEntityByName(namePtr, &hi, &lo);
			Marshal .FreeHGlobal(namePtr);

			if (hi == 0 && lo == 0) return null;

			return new Entity() { ID = new UUID { High = hi, Low = lo } };
		}

		public unsafe T? As<T>() where T : Entity {
			IntPtr gcHandlePtr = CoreAPI.API.Entity_GetScriptInstance(ID.High, ID.Low);

			if (gcHandlePtr != IntPtr.Zero) {
				GCHandle handle = GCHandle.FromIntPtr(gcHandlePtr);
				return handle.Target as T;
			}

			return null;
		}

		public unsafe void EmitParticles(int count) {
			CoreAPI.API.Entity_EmitParticles(ID.High, ID.Low, count);
		}

	}

}
