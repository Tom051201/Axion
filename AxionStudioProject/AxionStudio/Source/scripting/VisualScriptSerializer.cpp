#include "axpch.h"
#include "VisualScriptSerializer.h"

#include "AxionEngine/Source/core/YamlHelper.h"

namespace Axion {

	void VisualScriptSerializer::serialize(const VisualGraph& graph, const std::filesystem::path& filepath) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "VisualScript" << YAML::Value << graph.className;

		// -- Serialize Variables --
		out << YAML::Key << "Variables" << YAML::Value << YAML::BeginSeq;
		for (const auto& var : graph.variables) {
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << var.name;
			out << YAML::Key << "Type" << YAML::Value << pinTypeToString(var.type);

			out << YAML::Key << "FloatValue" << YAML::Value << var.floatValue;
			out << YAML::Key << "IntValue" << YAML::Value << var.intValue;
			out << YAML::Key << "BoolValue" << YAML::Value << var.boolValue;
			out << YAML::Key << "Vec3Value" << YAML::Value << var.vec3Value;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		// -- Serialize Nodes --
		out << YAML::Key << "Nodes" << YAML::Value << YAML::BeginSeq;
		for (const auto& node : graph.nodes) {
			out << YAML::BeginMap;
			out << YAML::Key << "ID" << YAML::Value << node.id;
			out << YAML::Key << "Type" << YAML::Value << nodeTypeToString(node.type);
			out << YAML::Key << "OperationType" << YAML::Value << pinTypeToString(node.operationType);
			out << YAML::Key << "Name" << YAML::Value << node.name;

			// Inputs
			out << YAML::Key << "Inputs" << YAML::Value << YAML::BeginSeq;
			for (const auto& pin : node.inputs) { serializePin(out, pin); }
			out << YAML::EndSeq;

			// Outputs
			out << YAML::Key << "Outputs" << YAML::Value << YAML::BeginSeq;
			for (const auto& pin : node.outputs) { serializePin(out, pin); }
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		// -- Serialize Links --
		out << YAML::Key << "Links" << YAML::Value << YAML::BeginSeq;
		for (const auto& link : graph.links) {
			out << YAML::BeginMap;
			out << YAML::Key << "ID" << YAML::Value << link.id;
			out << YAML::Key << "StartPin" << YAML::Value << link.startPinID;
			out << YAML::Key << "EndPin" << YAML::Value << link.endPinID;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
		fout.close();

		AX_CORE_LOG_INFO("Saved Visual Script to {}", filepath.string());
	}

	bool VisualScriptSerializer::deserialize(VisualGraph& outGraph, const std::filesystem::path& filepath) {
		if (!std::filesystem::exists(filepath)) return false;

		YAML::Node data;
		try {
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException& e) {
			AX_CORE_LOG_ERROR("Failed to load .axvs file '{}': {}", filepath.string(), e.what());
			return false;
		}

		if (!data["VisualScript"]) return false;

		outGraph.className = data["VisualScript"].as<std::string>();
		outGraph.nodes.clear();
		outGraph.links.clear();
		outGraph.variables.clear();

		auto vars = data["Variables"];
		if (vars) {
			for (auto varData : vars) {
				Variable var;
				var.name = varData["Name"].as<std::string>();
				var.type = pinTypeFromString(varData["Type"].as<std::string>());
				var.floatValue = varData["FloatValue"].as<float>();
				var.intValue = varData["IntValue"].as<int>();
				var.boolValue = varData["BoolValue"].as<bool>();
				var.vec3Value = varData["Vec3Value"].as<Vec3>();
				outGraph.variables.push_back(var);
			}
		}

		auto nodes = data["Nodes"];
		if (nodes) {
			for (auto nodeData : nodes) {
				Node node;
				node.id = nodeData["ID"].as<int>();
				node.type = nodeTypeFromString(nodeData["Type"].as<std::string>());
				node.name = nodeData["Name"].as<std::string>();

				if (nodeData["OperationType"]) {
					node.operationType = pinTypeFromString(nodeData["OperationType"].as<std::string>());
				}
				else {
					node.operationType = PinType::Float;
				}

				auto inputs = nodeData["Inputs"];
				if (inputs) {
					for (auto pinData : inputs) {
						Pin pin;
						pin.id = pinData["ID"].as<int>();
						pin.nodeID = pinData["NodeID"].as<int>();
						pin.name = pinData["Name"].as<std::string>();
						pin.kind = pinKindFromString(pinData["Kind"].as<std::string>());
						pin.type = pinTypeFromString(pinData["Type"].as<std::string>());

						pin.floatValue = pinData["FloatValue"].as<float>();
						pin.intValue = pinData["IntValue"] ? pinData["IntValue"].as<int>() : 0;
						pin.boolValue = pinData["BoolValue"].as<bool>();
						pin.stringValue = pinData["StringValue"].as<std::string>();
						pin.vec3Value = pinData["Vec3Value"].as<Vec3>();

						node.inputs.push_back(pin);
					}
				}

				auto outputs = nodeData["Outputs"];
				if (outputs) {
					for (auto pinData : outputs) {
						Pin pin;
						pin.id = pinData["ID"].as<int>();
						pin.nodeID = pinData["NodeID"].as<int>();
						pin.name = pinData["Name"].as<std::string>();
						pin.kind = pinKindFromString(pinData["Kind"].as<std::string>());
						pin.type = pinTypeFromString(pinData["Type"].as<std::string>());

						pin.floatValue = pinData["FloatValue"].as<float>();
						pin.intValue = pinData["IntValue"] ? pinData["IntValue"].as<int>() : 0;
						pin.boolValue = pinData["BoolValue"].as<bool>();
						pin.stringValue = pinData["StringValue"].as<std::string>();
						pin.vec3Value = pinData["Vec3Value"].as<Vec3>();

						node.outputs.push_back(pin);
					}
				}

				outGraph.nodes.push_back(node);
			}
		}

		auto links = data["Links"];
		if (links) {
			for (auto linkData : links) {
				Link link;
				link.id = linkData["ID"].as<int>();
				link.startPinID = linkData["StartPin"].as<int>();
				link.endPinID = linkData["EndPin"].as<int>();
				outGraph.links.push_back(link);
			}
		}

		return true;
	}

	void VisualScriptSerializer::serializePin(YAML::Emitter& out, const Pin& pin) {
		out << YAML::BeginMap;
		out << YAML::Key << "ID" << YAML::Value << pin.id;
		out << YAML::Key << "NodeID" << YAML::Value << pin.nodeID;
		out << YAML::Key << "Name" << YAML::Value << pin.name;
		out << YAML::Key << "Kind" << YAML::Value << pinKindToString(pin.kind);
		out << YAML::Key << "Type" << YAML::Value << pinTypeToString(pin.type);

		out << YAML::Key << "FloatValue" << YAML::Value << pin.floatValue;
		out << YAML::Key << "IntValue" << YAML::Value << pin.intValue;
		out << YAML::Key << "BoolValue" << YAML::Value << pin.boolValue;
		out << YAML::Key << "StringValue" << YAML::Value << pin.stringValue;
		out << YAML::Key << "Vec3Value" << YAML::Value << pin.vec3Value;
		out << YAML::EndMap;
	}

	PinKind VisualScriptSerializer::pinKindFromString(const std::string& str) {
		if (str == "Input") return PinKind::Input;
		else if (str == "Output") return PinKind::Output;
		else if (str == "None") return PinKind::None;

		AX_CORE_LOG_WARN("Unable converting string to pin kind!");
		return PinKind::None;
	}

	std::string VisualScriptSerializer::pinKindToString(const PinKind& kind) {
		switch (kind) {
			case PinKind::Input: { return "Input"; }
			case PinKind::Output: { return "Output"; }
			case PinKind::None: { return "None"; }
		}

		AX_CORE_LOG_WARN("Unable converting pin kind to string!");
		return "None";
	}

	PinType VisualScriptSerializer::pinTypeFromString(const std::string& str) {
		if (str == "Flow") return PinType::Flow;
		else if (str == "Bool") return PinType::Bool;
		else if (str == "Int") return PinType::Int;
		else if (str == "Float") return PinType::Float;
		else if (str == "String") return PinType::String;
		else if (str == "Vector3") return PinType::Vector3;
		else if (str == "Entity") return PinType::Entity;
		else if (str == "Key") return PinType::Key;
		else if (str == "MouseButton") return PinType::MouseButton;
		else if (str == "None") return PinType::None;

		AX_CORE_LOG_WARN("Unable converting string to pin type!");
		return PinType::None;
	}

	std::string VisualScriptSerializer::pinTypeToString(const PinType& type) {
		switch (type) {
			case PinType::Flow: { return "Flow"; }
			case PinType::Bool: { return "Bool"; }
			case PinType::Int: { return "Int"; }
			case PinType::Float: { return "Float"; }
			case PinType::String: { return "String"; }
			case PinType::Vector3: { return "Vector3"; }
			case PinType::Entity: { return "Entity"; }
			case PinType::Key: return "Key";
			case PinType::MouseButton: return "MouseButton";
			case PinType::None: { return "None"; }
		}

		AX_CORE_LOG_WARN("Unable converting pin type to string!");
		return "None";
	}

	NodeType VisualScriptSerializer::nodeTypeFromString(const std::string& str) {
		// -- EVENTS --
		if (str == "Event_OnCreate") return NodeType::Event_OnCreate;
		else if (str == "Event_OnDestroy") return NodeType::Event_OnDestroy;
		else if (str == "Event_OnUpdate") return NodeType::Event_OnUpdate;
		else if (str == "Event_OnCollisionEnter") return NodeType::Event_OnCollisionEnter;
		else if (str == "Event_OnCollisionExit") return NodeType::Event_OnCollisionExit;

		// -- ENTITY --
		else if (str == "Entity_Instantiate") return NodeType::Entity_Instantiate;
		else if (str == "Entity_InstantiatePrefab") return NodeType::Entity_InstantiatePrefab;
		else if (str == "Entity_Destroy") return NodeType::Entity_Destroy;
		else if (str == "Entity_FindByName") return NodeType::Entity_FindByName;
		else if (str == "Entity_EmitParticles") return NodeType::Entity_EmitParticles;

		// -- TRANSFORM --
		else if (str == "Transform_GetPosition") return NodeType::Transform_GetPosition;
		else if (str == "Transform_SetPosition") return NodeType::Transform_SetPosition;
		else if (str == "Transform_GetRotation") return NodeType::Transform_GetRotation;
		else if (str == "Transform_SetRotation") return NodeType::Transform_SetRotation;
		else if (str == "Transform_GetScale") return NodeType::Transform_GetScale;
		else if (str == "Transform_SetScale") return NodeType::Transform_SetScale;
		else if (str == "Transform_GetForward") return NodeType::Transform_GetForward;
		else if (str == "Transform_GetRight") return NodeType::Transform_GetRight;
		else if (str == "Transform_GetUp") return NodeType::Transform_GetUp;

		// -- RIGIDBODY --
		else if (str == "RigidBody_AddForce") return NodeType::RigidBody_AddForce;
		else if (str == "RigidBody_AddTorque") return NodeType::RigidBody_AddTorque;
		else if (str == "RigidBody_AddImpulse") return NodeType::RigidBody_AddImpulse;
		else if (str == "RigidBody_AddRadialImpulse") return NodeType::RigidBody_AddRadialImpulse;
		else if (str == "RigidBody_GetLinearVelocity") return NodeType::RigidBody_GetLinearVelocity;
		else if (str == "RigidBody_SetLinearVelocity") return NodeType::RigidBody_SetLinearVelocity;
		else if (str == "RigidBody_GetAngularVelocity") return NodeType::RigidBody_GetAngularVelocity;
		else if (str == "RigidBody_SetAngularVelocity") return NodeType::RigidBody_SetAngularVelocity;
		else if (str == "RigidBody_GetMass") return NodeType::RigidBody_GetMass;
		else if (str == "RigidBody_SetMass") return NodeType::RigidBody_SetMass;

		// -- INPUT --
		else if (str == "Input_IsKeyPressed") return NodeType::Input_IsKeyPressed;
		else if (str == "Input_IsMouseButtonPressed") return NodeType::Input_IsMouseButtonPressed;

		// -- AUDIO --
		else if (str == "Audio_Play") return NodeType::Audio_Play;
		else if (str == "Audio_Stop") return NodeType::Audio_Stop;
		else if (str == "Audio_GetVolume") return NodeType::Audio_GetVolume;
		else if (str == "Audio_SetVolume") return NodeType::Audio_SetVolume;

		// -- ANIMATOR --
		else if (str == "Animator_Play") return NodeType::Animator_Play;
		else if (str == "Animator_Stop") return NodeType::Animator_Stop;
		else if (str == "Animator_IsPlaying") return NodeType::Animator_IsPlaying;

		// -- LOGIC --
		else if (str == "Logic_Branch") return NodeType::Logic_Branch;
		else if (str == "Logic_Sequence") return NodeType::Logic_Sequence;
		else if (str == "Logic_And") return NodeType::Logic_And;
		else if (str == "Logic_Or") return NodeType::Logic_Or;

		// -- MATH --
		else if (str == "Math_Add") return NodeType::Math_Add;
		else if (str == "Math_Subtract") return NodeType::Math_Subtract;
		else if (str == "Math_Multiply") return NodeType::Math_Multiply;
		else if (str == "Math_Divide") return NodeType::Math_Divide;
		else if (str == "Math_Equal") return NodeType::Math_Equal;
		else if (str == "Math_Greater") return NodeType::Math_Greater;
		else if (str == "Math_Less") return NodeType::Math_Less;
		else if (str == "Math_MakeVector3") return NodeType::Math_MakeVector3;
		else if (str == "Math_BreakVector3") return NodeType::Math_BreakVector3;

		// -- VARIABLES --
		else if (str == "Variable_GetFloat") return NodeType::Variable_GetFloat;
		else if (str == "Variable_SetFloat") return NodeType::Variable_SetFloat;
		else if (str == "Variable_GetInt") return NodeType::Variable_GetInt;
		else if (str == "Variable_SetInt") return NodeType::Variable_SetInt;
		else if (str == "Variable_GetBool") return NodeType::Variable_GetBool;
		else if (str == "Variable_SetBool") return NodeType::Variable_SetBool;
		else if (str == "Variable_GetVector3") return NodeType::Variable_GetVector3;
		else if (str == "Variable_SetVector3") return NodeType::Variable_SetVector3;

		else if (str == "None") return NodeType::None;

		AX_CORE_LOG_WARN("Unable converting string to node type: {}", str);
		return NodeType::None;
	}

	std::string VisualScriptSerializer::nodeTypeToString(const NodeType& type) {
		switch (type) {
			// -- EVENTS --
			case NodeType::Event_OnCreate: return "Event_OnCreate";
			case NodeType::Event_OnDestroy: return "Event_OnDestroy";
			case NodeType::Event_OnUpdate: return "Event_OnUpdate";
			case NodeType::Event_OnCollisionEnter: return "Event_OnCollisionEnter";
			case NodeType::Event_OnCollisionExit: return "Event_OnCollisionExit";

			// -- ENTITY --
			case NodeType::Entity_Instantiate: return "Entity_Instantiate";
			case NodeType::Entity_InstantiatePrefab: return "Entity_InstantiatePrefab";
			case NodeType::Entity_Destroy: return "Entity_Destroy";
			case NodeType::Entity_FindByName: return "Entity_FindByName";
			case NodeType::Entity_EmitParticles: return "Entity_EmitParticles";

			// -- TRANSFORM --
			case NodeType::Transform_GetPosition: return "Transform_GetPosition";
			case NodeType::Transform_SetPosition: return "Transform_SetPosition";
			case NodeType::Transform_GetRotation: return "Transform_GetRotation";
			case NodeType::Transform_SetRotation: return "Transform_SetRotation";
			case NodeType::Transform_GetScale: return "Transform_GetScale";
			case NodeType::Transform_SetScale: return "Transform_SetScale";
			case NodeType::Transform_GetForward: return "Transform_GetForward";
			case NodeType::Transform_GetRight: return "Transform_GetRight";
			case NodeType::Transform_GetUp: return "Transform_GetUp";

			// -- RIGIDBODY --
			case NodeType::RigidBody_AddForce: return "RigidBody_AddForce";
			case NodeType::RigidBody_AddTorque: return "RigidBody_AddTorque";
			case NodeType::RigidBody_AddImpulse: return "RigidBody_AddImpulse";
			case NodeType::RigidBody_AddRadialImpulse: return "RigidBody_AddRadialImpulse";
			case NodeType::RigidBody_GetLinearVelocity: return "RigidBody_GetLinearVelocity";
			case NodeType::RigidBody_SetLinearVelocity: return "RigidBody_SetLinearVelocity";
			case NodeType::RigidBody_GetAngularVelocity: return "RigidBody_GetAngularVelocity";
			case NodeType::RigidBody_SetAngularVelocity: return "RigidBody_SetAngularVelocity";
			case NodeType::RigidBody_GetMass: return "RigidBody_GetMass";
			case NodeType::RigidBody_SetMass: return "RigidBody_SetMass";

			// -- INPUT --
			case NodeType::Input_IsKeyPressed: return "Input_IsKeyPressed";
			case NodeType::Input_IsMouseButtonPressed: return "Input_IsMouseButtonPressed";

			// -- AUDIO --
			case NodeType::Audio_Play: return "Audio_Play";
			case NodeType::Audio_Stop: return "Audio_Stop";
			case NodeType::Audio_GetVolume: return "Audio_GetVolume";
			case NodeType::Audio_SetVolume: return "Audio_SetVolume";

			// -- ANIMATOR --
			case NodeType::Animator_Play: return "Animator_Play";
			case NodeType::Animator_Stop: return "Animator_Stop";
			case NodeType::Animator_IsPlaying: return "Animator_IsPlaying";

			// -- LOGIC --
			case NodeType::Logic_Branch: return "Logic_Branch";
			case NodeType::Logic_Sequence: return "Logic_Sequence";
			case NodeType::Logic_And: return "Logic_And";
			case NodeType::Logic_Or: return "Logic_Or";

			// -- MATH --
			case NodeType::Math_Add: return "Math_Add";
			case NodeType::Math_Subtract: return "Math_Subtract";
			case NodeType::Math_Multiply: return "Math_Multiply";
			case NodeType::Math_Divide: return "Math_Divide";
			case NodeType::Math_Equal: return "Math_Equal";
			case NodeType::Math_Greater: return "Math_Greater";
			case NodeType::Math_Less: return "Math_Less";
			case NodeType::Math_MakeVector3: return "Math_MakeVector3";
			case NodeType::Math_BreakVector3: return "Math_BreakVector3";

			// -- VARIABLES --
			case NodeType::Variable_GetFloat: return "Variable_GetFloat";
			case NodeType::Variable_SetFloat: return "Variable_SetFloat";
			case NodeType::Variable_GetInt: return "Variable_GetInt";
			case NodeType::Variable_SetInt: return "Variable_SetInt";

			case NodeType::Variable_GetBool: return "Variable_GetBool";
			case NodeType::Variable_SetBool: return "Variable_SetBool";
			case NodeType::Variable_GetVector3: return "Variable_GetVector3";
			case NodeType::Variable_SetVector3: return "Variable_SetVector3";

			case NodeType::None: return "None";
		}

		AX_CORE_LOG_WARN("Unable converting node type to string!");
		return "None";
	}

}
