using System;
using System.Runtime.InteropServices;

namespace AxionScriptCore {

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

}
