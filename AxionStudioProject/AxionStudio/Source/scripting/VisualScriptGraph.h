#pragma once
#include "axpch.h"

#include "AxionEngine/Source/core/Math.h"

namespace Axion {

	enum class PinKind { None, Input, Output };

	enum class PinType { None, Flow, Bool, Int, Float, String, Vector3, Entity, Key, MouseButton };

	enum class NodeType {
		None = 0,

		// -- EVENT --
		Event_OnCreate,
		Event_OnDestroy,
		Event_OnUpdate,
		Event_OnCollisionEnter,
		Event_OnCollisionExit,

		// -- ENTITY --
		Entity_Instantiate,
		Entity_InstantiatePrefab,
		Entity_Destroy,
		Entity_FindByName,
		Entity_EmitParticles,

		// -- TRANSFORM --
		Transform_GetPosition,
		Transform_SetPosition,
		Transform_GetRotation,
		Transform_SetRotation,
		Transform_GetScale,
		Transform_SetScale,
		Transform_GetForward,
		Transform_GetRight,
		Transform_GetUp,

		// -- RIGIDBODY --
		RigidBody_AddForce,
		RigidBody_AddTorque,
		RigidBody_AddImpulse,
		RigidBody_AddRadialImpulse,
		RigidBody_GetLinearVelocity,
		RigidBody_SetLinearVelocity,
		RigidBody_GetAngularVelocity,
		RigidBody_SetAngularVelocity,
		RigidBody_GetMass,
		RigidBody_SetMass,

		// -- INPUT --
		Input_IsKeyPressed,
		Input_IsMouseButtonPressed,

		// -- AUDIO --
		Audio_Play,
		Audio_Stop,
		Audio_GetVolume,
		Audio_SetVolume,

		// -- ANIMATOR --
		Animator_Play,
		Animator_Stop,
		Animator_IsPlaying,

		// -- LOGIC --
		Logic_Branch,
		Logic_Sequence,
		Logic_And,
		Logic_Or,

		// -- MATH --
		Math_Add,
		Math_Subtract,
		Math_Multiply,
		Math_Divide,
		Math_Equal,
		Math_Greater,
		Math_Less,
		Math_MakeVector3,
		Math_BreakVector3,

		// -- VARIABLES --
		Variable_GetFloat,
		Variable_SetFloat,
		Variable_GetInt,
		Variable_SetInt,
		Variable_GetBool,
		Variable_SetBool,
		Variable_GetVector3,
		Variable_SetVector3
	};

	struct Pin {
		int id;
		int nodeID;
		std::string name;
		PinKind kind;
		PinType type;

		float floatValue = 0.0f;
		int intValue = 0;
		Vec3 vec3Value = Vec3::zero();
		bool boolValue = false;
		std::string stringValue = "";
	};

	struct Node {
		int id;
		NodeType type;
		std::string name;
		std::vector<Pin> inputs;
		std::vector<Pin> outputs;

		PinType operationType = PinType::Float;
	};

	struct Link {
		int id;
		int startPinID;
		int endPinID;
	};

	struct Variable {
		std::string name = "NewVar";
		PinType type = PinType::Float;

		float floatValue = 0.0f;
		int intValue = 0;
		bool boolValue = false;
		Vec3 vec3Value = Vec3::zero();
	};

	struct VisualGraph {
		std::string className;
		std::vector<Node> nodes;
		std::vector<Link> links;
		std::vector<Variable> variables;
	};

}
