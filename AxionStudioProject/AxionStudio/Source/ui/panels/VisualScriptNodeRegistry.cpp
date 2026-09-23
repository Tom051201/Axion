#include "studiopch.h"
#include "VisualScriptNodeRegistry.h"

namespace Axion {

	const std::vector<NodeType>& VSNodeRegistry::getAllNodeTypes() {
		static std::vector<NodeType> s_AllNodes = {
			NodeType::Event_OnCreate, NodeType::Event_OnDestroy, NodeType::Event_OnUpdate, NodeType::Event_OnCollisionEnter, NodeType::Event_OnCollisionExit,
			NodeType::Scene_Load, NodeType::Scene_Save, NodeType::Scene_IsLoading,
			NodeType::Entity_Instantiate, NodeType::Entity_InstantiatePrefab, NodeType::Entity_Destroy, NodeType::Entity_FindByName, NodeType::Entity_EmitParticles,
			NodeType::Transform_GetPosition, NodeType::Transform_GetRotation, NodeType::Transform_GetScale, NodeType::Transform_GetForward, NodeType::Transform_GetRight, NodeType::Transform_GetUp, NodeType::Transform_SetPosition, NodeType::Transform_SetRotation, NodeType::Transform_SetScale,
			NodeType::RigidBody_AddForce, NodeType::RigidBody_AddTorque, NodeType::RigidBody_AddImpulse, NodeType::RigidBody_AddRadialImpulse, NodeType::RigidBody_GetLinearVelocity, NodeType::RigidBody_SetLinearVelocity, NodeType::RigidBody_GetAngularVelocity, NodeType::RigidBody_SetAngularVelocity, NodeType::RigidBody_GetMass, NodeType::RigidBody_SetMass,
			NodeType::Input_IsKeyPressed, NodeType::Input_IsMouseButtonPressed,
			NodeType::Audio_Play, NodeType::Audio_Stop, NodeType::Audio_GetVolume, NodeType::Audio_SetVolume,
			NodeType::Animator_Play, NodeType::Animator_Stop, NodeType::Animator_IsPlaying,
			NodeType::Network_IsLocalPlayer, NodeType::Network_SendEvent,
			NodeType::Logic_Branch, NodeType::Logic_Sequence, NodeType::Logic_And, NodeType::Logic_Or,
			NodeType::Math_Add, NodeType::Math_Subtract, NodeType::Math_Multiply, NodeType::Math_Divide, NodeType::Math_Equal, NodeType::Math_Greater, NodeType::Math_Less, NodeType::Math_MakeVector3, NodeType::Math_BreakVector3,
			NodeType::Variable_Get, NodeType::Variable_Set,
			NodeType::CharacterController_Move, NodeType::CharacterController_IsGrounded,
		};
		return s_AllNodes;
	}

	const VSNodeDef& VSNodeRegistry::getNodeDef(NodeType type) {
		static VSNodeDef emptyDef;
		switch (type) {
		// -- EVENTS --
			case NodeType::Event_OnCreate: { static VSNodeDef d = { "On Create", "Events", {}, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Event_OnDestroy: { static VSNodeDef d = { "On Destroy", "Events", {}, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Event_OnUpdate: { static VSNodeDef d = { "On Update", "Events", {}, { {"Next", PinType::Flow}, {"Delta Time", PinType::Float} } }; return d; }
			case NodeType::Event_OnCollisionEnter: { static VSNodeDef d = { "On Collision Enter", "Events", {}, { {"Next", PinType::Flow}, {"Other Entity", PinType::Entity} } }; return d; }
			case NodeType::Event_OnCollisionExit: { static VSNodeDef d = { "On Collision Exit", "Events", {}, { {"Next", PinType::Flow}, {"Other Entity", PinType::Entity} } }; return d; }

		// -- SCENE --
			case NodeType::Scene_Load: { static VSNodeDef d = { "Load Scene", "Scene", { {"Execute", PinType::Flow}, {"File Path", PinType::String} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Scene_Save: { static VSNodeDef d = { "Save Scene", "Scene", { {"Execute", PinType::Flow}, {"File Path", PinType::String} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Scene_IsLoading: { static VSNodeDef d = { "Is Loading Scene", "Scene", {}, { {"Result", PinType::Bool} } }; return d; }

		// -- ENTITY --
			case NodeType::Entity_Instantiate: { static VSNodeDef d = { "Instantiate Entity", "Entity", { {"Execute", PinType::Flow}, {"Name", PinType::String} }, { {"Next", PinType::Flow}, {"Entity", PinType::Entity} } }; return d; }
			case NodeType::Entity_InstantiatePrefab: { static VSNodeDef d = { "Instantiate Prefab", "Entity", { {"Execute", PinType::Flow}, {"File Path", PinType::String} }, { {"Next", PinType::Flow}, {"Entity", PinType::Entity} } }; return d; }
			case NodeType::Entity_Destroy: { static VSNodeDef d = { "Destroy Entity", "Entity", { {"Execute", PinType::Flow}, {"Target", PinType::Entity} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Entity_FindByName: { static VSNodeDef d = { "Find Entity by Name", "Entity", { {"Name", PinType::String} }, { {"Entity", PinType::Entity} } }; return d; }
			case NodeType::Entity_EmitParticles: { static VSNodeDef d = { "Emit Particles", "Entity", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Count", PinType::Int} }, { {"Next", PinType::Flow} } }; return d; }

		// -- TRANSFORM --
			case NodeType::Transform_GetPosition: { static VSNodeDef d = { "Get Position", "Transform", { {"Target", PinType::Entity} }, { {"Result", PinType::Vector3} } }; return d; }
			case NodeType::Transform_GetRotation: { static VSNodeDef d = { "Get Rotation", "Transform", { {"Target", PinType::Entity} }, { {"Result", PinType::Vector3} } }; return d; }
			case NodeType::Transform_GetScale: { static VSNodeDef d = { "Get Scale", "Transform", { {"Target", PinType::Entity} }, { {"Result", PinType::Vector3} } }; return d; }
			case NodeType::Transform_GetForward: { static VSNodeDef d = { "Get Forward Vector", "Transform", { {"Target", PinType::Entity} }, { {"Result", PinType::Vector3} } }; return d; }
			case NodeType::Transform_GetRight: { static VSNodeDef d = { "Get Right Vector", "Transform", { {"Target", PinType::Entity} }, { {"Result", PinType::Vector3} } }; return d; }
			case NodeType::Transform_GetUp: { static VSNodeDef d = { "Get Up Vector", "Transform", { {"Target", PinType::Entity} }, { {"Result", PinType::Vector3} } }; return d; }
			case NodeType::Transform_SetPosition: { static VSNodeDef d = { "Set Position", "Transform", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Value", PinType::Vector3} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Transform_SetRotation: { static VSNodeDef d = { "Set Rotation", "Transform", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Value", PinType::Vector3} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Transform_SetScale: { static VSNodeDef d = { "Set Scale", "Transform", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Value", PinType::Vector3} }, { {"Next", PinType::Flow} } }; return d; }

		// -- RIGIDBODY --
			case NodeType::RigidBody_AddForce: { static VSNodeDef d = { "Add Force", "Rigid Body", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Force", PinType::Vector3} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::RigidBody_AddTorque: { static VSNodeDef d = { "Add Torque", "Rigid Body", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Torque", PinType::Vector3} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::RigidBody_AddImpulse: { static VSNodeDef d = { "Add Impulse", "Rigid Body", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Force", PinType::Vector3} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::RigidBody_AddRadialImpulse: { static VSNodeDef d = { "Add Radial Impulse", "Rigid Body", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Origin", PinType::Vector3}, {"Radius", PinType::Float}, {"Strength", PinType::Float} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::RigidBody_GetLinearVelocity: { static VSNodeDef d = { "Get Linear Velocity", "Rigid Body", { {"Target", PinType::Entity} }, { {"Velocity", PinType::Vector3} } }; return d; }
			case NodeType::RigidBody_SetLinearVelocity: { static VSNodeDef d = { "Set Linear Velocity", "Rigid Body", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Velocity", PinType::Vector3} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::RigidBody_GetAngularVelocity: { static VSNodeDef d = { "Get Angular Velocity", "Rigid Body", { {"Target", PinType::Entity} }, { {"Velocity", PinType::Vector3} } }; return d; }
			case NodeType::RigidBody_SetAngularVelocity: { static VSNodeDef d = { "Set Angular Velocity", "Rigid Body", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Velocity", PinType::Vector3} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::RigidBody_GetMass: { static VSNodeDef d = { "Get Mass", "Rigid Body", { {"Target", PinType::Entity} }, { {"Mass", PinType::Float} } }; return d; }
			case NodeType::RigidBody_SetMass: { static VSNodeDef d = { "Set Mass", "Rigid Body", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Mass", PinType::Float} }, { {"Next", PinType::Flow} } }; return d; }

		// -- CHARACTER CONTROLLER --
			case NodeType::CharacterController_Move: { static VSNodeDef d = { "Move Character", "Character Controller", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Displacement", PinType::Vector3}, {"Timestep", PinType::Float} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::CharacterController_IsGrounded: { static VSNodeDef d = { "Is Grounded", "Character Controller", { {"Target", PinType::Entity} }, { {"Result", PinType::Bool} } }; return d; }

		// -- INPUT --
			case NodeType::Input_IsKeyPressed: { static VSNodeDef d = { "Is Key Pressed", "Input", { {"Key", PinType::Key, "Space"} }, { {"Result", PinType::Bool} } }; return d; }
			case NodeType::Input_IsMouseButtonPressed: { static VSNodeDef d = { "Is Mouse Button Pressed", "Input", { {"Button", PinType::MouseButton, "Left"} }, { {"Result", PinType::Bool} } }; return d; }

		// -- AUDIO --
			case NodeType::Audio_Play: { static VSNodeDef d = { "Play Audio", "Audio", { {"Execute", PinType::Flow} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Audio_Stop: { static VSNodeDef d = { "Stop Audio", "Audio", { {"Execute", PinType::Flow}, {"Target", PinType::Entity} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Audio_GetVolume: { static VSNodeDef d = { "Get Volume", "Audio", { {"Target", PinType::Entity} }, { {"Volume", PinType::Float} } }; return d; }
			case NodeType::Audio_SetVolume: { static VSNodeDef d = { "Set Volume", "Audio", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Volume", PinType::Float} }, { {"Next", PinType::Flow} } }; return d; }

		// -- ANIMATOR --
			case NodeType::Animator_Play: { static VSNodeDef d = { "Play Animation", "Animator", { {"Execute", PinType::Flow}, {"Target", PinType::Entity} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Animator_Stop: { static VSNodeDef d = { "Stop Animation", "Animator", { {"Execute", PinType::Flow}, {"Target", PinType::Entity} }, { {"Next", PinType::Flow} } }; return d; }
			case NodeType::Animator_IsPlaying: { static VSNodeDef d = { "Is Playing", "Animator", { {"Target", PinType::Entity} }, { {"Result", PinType::Bool} } }; return d; }

		// -- NETWORK --
			case NodeType::Network_IsLocalPlayer: { static VSNodeDef d = { "Is Local Player", "Network", { {"Target", PinType::Entity} }, { {"Result", PinType::Bool} } }; return d; }
			case NodeType::Network_SendEvent: { static VSNodeDef d = { "Send Network Event", "Network", { {"Execute", PinType::Flow}, {"Target", PinType::Entity}, {"Event ID", PinType::Int} }, { {"Next", PinType::Flow} } }; return d; }

		// -- LOGIC --
			case NodeType::Logic_Branch: { static VSNodeDef d = { "Branch", "Logic", { {"Execute", PinType::Flow}, {"Condition", PinType::Bool} }, { {"True", PinType::Flow}, {"False", PinType::Flow} } }; return d; }
			case NodeType::Logic_Sequence: { static VSNodeDef d = { "Sequence", "Logic", { {"Execute", PinType::Flow} }, { {"Then 0", PinType::Flow}, {"Then 1", PinType::Flow}, {"Then 2", PinType::Flow}, {"Then 3", PinType::Flow}, {"Then 4", PinType::Flow} } }; return d; }
			case NodeType::Logic_And: { static VSNodeDef d = { "And (&&)", "Logic", { {"Condition A", PinType::Bool}, {"Condition B", PinType::Bool} }, { {"True", PinType::Bool} } }; return d; }
			case NodeType::Logic_Or: { static VSNodeDef d = { "Or (||)", "Logic", { {"Condition A", PinType::Bool}, {"Condition B", PinType::Bool} }, { {"True", PinType::Bool} } }; return d; }

		// -- MATH --
			case NodeType::Math_Add: { static VSNodeDef d = { "Add (+)", "Math", { {"A", PinType::Float}, {"B", PinType::Float} }, { {"Result", PinType::Float} } }; return d; }
			case NodeType::Math_Subtract: { static VSNodeDef d = { "Subtract (-)", "Math", { {"A", PinType::Float}, {"B", PinType::Float} }, { {"Result", PinType::Float} } }; return d; }
			case NodeType::Math_Multiply: { static VSNodeDef d = { "Multiply (*)", "Math", { {"A", PinType::Float}, {"B", PinType::Float} }, { {"Result", PinType::Float} } }; return d; }
			case NodeType::Math_Divide: { static VSNodeDef d = { "Divide (/)", "Math", { {"A", PinType::Float}, {"B", PinType::Float} }, { {"Result", PinType::Float} } }; return d; }
			case NodeType::Math_Equal: { static VSNodeDef d = { "Equal (==)", "Math", { {"A", PinType::Float}, {"B", PinType::Float} }, { {"Result", PinType::Bool} } }; return d; }
			case NodeType::Math_Greater: { static VSNodeDef d = { "Greater (>)", "Math", { {"A", PinType::Float}, {"B", PinType::Float} }, { {"Result", PinType::Bool} } }; return d; }
			case NodeType::Math_Less: { static VSNodeDef d = { "Less (<)", "Math", { {"A", PinType::Float}, {"B", PinType::Float} }, { {"Result", PinType::Bool} } }; return d; }
			case NodeType::Math_MakeVector3: { static VSNodeDef d = { "Make Vector3", "Math", { {"X", PinType::Float}, {"Y", PinType::Float}, {"Z", PinType::Float} }, { {"Vector", PinType::Vector3} } }; return d; }
			case NodeType::Math_BreakVector3: { static VSNodeDef d = { "Break Vector3", "Math", { {"Vector", PinType::Vector3} }, { {"X", PinType::Float}, {"Y", PinType::Float}, {"Z", PinType::Float} } }; return d; }

		// -- VARIABLES --
			case NodeType::Variable_Get: { static VSNodeDef d = { "Get Variable", "Variables", { {"Name", PinType::String} }, { {"Value", PinType::Float} } }; return d; }
			case NodeType::Variable_Set: { static VSNodeDef d = { "Set Variable", "Variables", { {"Execute", PinType::Flow}, {"Name", PinType::String}, {"Value", PinType::Float} }, { {"Next", PinType::Flow} } }; return d; }

			default: return emptyDef;
		}
	}

	Silica::Color VSNodeRegistry::getNodeColor(NodeType type) {
		if (type >= NodeType::Event_OnCreate && type <= NodeType::Event_OnCollisionExit) return Silica::Color(140, 21, 21);
		if (type >= NodeType::Scene_Load && type <= NodeType::Scene_IsLoading) return Silica::Color(156, 39, 176);
		if (type >= NodeType::Entity_Instantiate && type <= NodeType::Entity_EmitParticles) return Silica::Color(30, 136, 229);
		if (type >= NodeType::Transform_GetPosition && type <= NodeType::Transform_GetUp) return Silica::Color(230, 81, 0);
		if (type >= NodeType::RigidBody_AddForce && type <= NodeType::RigidBody_SetMass) return Silica::Color(67, 160, 71);
		if (type >= NodeType::Input_IsKeyPressed && type <= NodeType::Input_IsMouseButtonPressed) return Silica::Color(123, 31, 162);
		if (type >= NodeType::Audio_Play && type <= NodeType::Audio_SetVolume) return Silica::Color(121, 85, 72);
		if (type >= NodeType::Animator_Play && type <= NodeType::Animator_IsPlaying) return Silica::Color(0, 172, 193);
		if (type >= NodeType::Network_IsLocalPlayer && type <= NodeType::Network_SendEvent) return Silica::Color(0, 150, 136);
		if (type >= NodeType::Logic_Branch && type <= NodeType::Logic_Or) return Silica::Color(96, 125, 139);
		if (type >= NodeType::Math_Add && type <= NodeType::Math_BreakVector3) return Silica::Color(85, 139, 47);
		if (type == NodeType::Variable_Get || type == NodeType::Variable_Set) return Silica::Color(139, 195, 74);
		if (type == NodeType::CharacterController_Move || type == NodeType::CharacterController_IsGrounded) return Silica::Color(0, 188, 212);
		return Silica::Color(80, 80, 80);
	}

	Silica::Color VSNodeRegistry::getPinColor(PinType type) {
		switch (type) {
			case PinType::Flow: return Silica::Color(255, 255, 255);
			case PinType::Bool: return Silica::Color(128, 0, 0);
			case PinType::Int: return Silica::Color(0, 191, 165);
			case PinType::Float: return Silica::Color(139, 195, 74);
			case PinType::String: return Silica::Color(233, 30, 99);
			case PinType::Vector3: return Silica::Color(251, 192, 45);
			case PinType::Entity: return Silica::Color(100, 181, 246);
			case PinType::Key: return Silica::Color(156, 39, 176);
			case PinType::MouseButton: return Silica::Color(156, 39, 176);
			default: return Silica::Color(200, 200, 200);
		}
	}

}
