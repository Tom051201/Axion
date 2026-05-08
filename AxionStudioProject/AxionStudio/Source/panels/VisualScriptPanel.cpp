#include "VisualScriptPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/scripting/VisualScriptSerializer.h"
#include "AxionStudio/Source/scripting/VisualScriptCompiler.h"
#include "AxionStudio/Source/core/EditorResourceManager.h"

namespace Axion {

	namespace ed = ax::NodeEditor;

	static const char* s_KeyNames[] = {
		"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
		"Zero", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine",
		"Semicolon", "Equal", "Comma", "Minus", "Period", "Slash", "GraveAccent", "LeftBracket", "Backslash", "RightBracket", "Apostrophe",
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
		"Left", "Right", "Up", "Down", "PageUp", "PageDown", "Home", "End", "Insert", "Delete",
		"LeftShift", "RightShift", "LeftControl", "RightControl", "LeftAlt", "RightAlt", "CapsLock", "NumLock", "ScrollLock", "PrintScreen", "Pause",
		"Space", "Enter", "Escape", "Tab", "Backspace",
		"Numpad0", "Numpad1", "Numpad2", "Numpad3", "Numpad4", "Numpad5", "Numpad6", "Numpad7", "Numpad8", "Numpad9",
		"NumpadDecimal", "NumpadDivide", "NumpadMultiply", "NumpadSubtract", "NumpadAdd"
	};

	static const char* s_MouseButtonNames[] = {
		"Left", "Right", "Middle", "X1", "X2"
	};

	VisualScriptPanel::VisualScriptPanel(const std::string& name)
		: Panel(name) {}

	VisualScriptPanel::~VisualScriptPanel() {
		shutdown();
	}

	void VisualScriptPanel::setup() {
		EditorResourceManager::loadIcon("vsp_header_bg", "AxionStudio/Resources/HeaderBackground.png");
	}

	void VisualScriptPanel::shutdown() {
		if (m_editorContext) ed::DestroyEditor(m_editorContext);
		m_editorContext = nullptr;
	}

	void VisualScriptPanel::setContext(const VisualGraph& graph, const std::filesystem::path& filePath) {
		m_activeGraph = graph;
		m_currentFilePath = filePath;

		m_nextNodeId = 1;
		m_nextPinId = 1000;
		m_nextLinkId = 100;

		for (const auto& node : m_activeGraph.nodes) {
			if (node.id >= m_nextNodeId) m_nextNodeId = node.id + 1;

			for (const auto& pin : node.inputs) {
				if (pin.id >= m_nextPinId) m_nextPinId = pin.id + 1;
			}
			for (const auto& pin : node.outputs) {
				if (pin.id >= m_nextPinId) m_nextPinId = pin.id + 1;
			}
		}

		for (const auto& link : m_activeGraph.links) {
			if (link.id >= m_nextLinkId) m_nextLinkId = link.id + 1;
		}
	}

	void VisualScriptPanel::openScript(const std::filesystem::path& filePath) {
		// -- Save the active Script --
		if (!m_currentFilePath.empty() && !m_activeGraph.nodes.empty()) {
			compileAndSave();
		}

		// -- Load new Script --
		VisualGraph loadedGraph;
		if (VisualScriptSerializer::deserialize(loadedGraph, filePath)) {
			setContext(loadedGraph, filePath);

			if (m_editorContext) {
				ed::DestroyEditor(m_editorContext);
				m_editorContext = nullptr;
			}

			std::filesystem::path layoutPath = filePath.parent_path() / (filePath.stem().string() + "_layout.json");
			m_currentLayoutFilePath = layoutPath.string();

			m_config = {};
			m_config.SettingsFile = m_currentLayoutFilePath.c_str();
			m_editorContext = ed::CreateEditor(&m_config);

			ed::SetCurrentEditor(m_editorContext);
			ed::Style& style = ed::GetStyle();
			style.NodeRounding = 6.0f;
			style.NodeBorderWidth = 1.5;
			style.HoveredNodeBorderWidth = 2.5f;
			style.SelectedNodeBorderWidth = 3.0f;
			style.PinRounding = 4.0f;
			style.LinkStrength = 4.0f;
			style.Colors[ed::StyleColor_NodeBg] = ImColor(32, 32, 32, 240);
			style.Colors[ed::StyleColor_NodeBorder] = ImColor(255, 255, 255, 40);
			ed::SetCurrentEditor(nullptr);

			AX_CORE_LOG_TRACE("Opened Visual Script: {}", filePath.filename().string());
		}
		else {
			AX_CORE_LOG_ERROR("Failed to open Visual Script: {}", filePath.string());
		}
	}

	void VisualScriptPanel::closeActiveScript() {
		// -- Destroy Layout Context --
		if (m_editorContext) {
			ed::DestroyEditor(m_editorContext);
			m_editorContext = nullptr;
		}

		// -- Clear Memory --
		m_activeGraph = VisualGraph();
		m_currentFilePath.clear();
		m_currentLayoutFilePath.clear();
	}

	void VisualScriptPanel::onGuiRender() {
		ImGui::Begin("Visual Script Editor");

		if (!m_activeGraph.className.empty()) {

			drawToolbar();

			// ----- Left Variables Sidebar -----
			ImGui::BeginChild("LeftPanel", ImVec2(250, 0), true);

			ImGui::TextUnformatted("GRAPH VARIABLES");
			ImGui::SameLine(ImGui::GetWindowWidth() - 30);
			if (ImGui::Button("+##AddVar")) {
				Variable newVar;
				newVar.name = "Var_" + std::to_string(m_activeGraph.variables.size());
				newVar.type = PinType::Float;
				m_activeGraph.variables.push_back(newVar);
			}
			ImGui::Separator();

			for (int i = 0; i < m_activeGraph.variables.size(); i++) {
				auto& var = m_activeGraph.variables[i];
				ImGui::PushID(i);

				// 1. Variable Name
				ImGui::SetNextItemWidth(120.0f);
				ImGui::InputText("##Name", &var.name);
				ImGui::SameLine();

				// 2. Variable Type
				ImGui::SetNextItemWidth(80.0f);
				std::string typeStr = "Float";
				if (var.type == PinType::Int) typeStr = "Int";
				else if (var.type == PinType::Vector3) typeStr = "Vector3";
				else if (var.type == PinType::Bool) typeStr = "Bool";

				if (ImGui::BeginCombo("##Type", typeStr.c_str())) {
					if (ImGui::Selectable("Float", var.type == PinType::Float)) var.type = PinType::Float;
					if (ImGui::Selectable("Int", var.type == PinType::Int)) var.type = PinType::Int;
					if (ImGui::Selectable("Vector3", var.type == PinType::Vector3)) var.type = PinType::Vector3;
					if (ImGui::Selectable("Bool", var.type == PinType::Bool)) var.type = PinType::Bool;
					ImGui::EndCombo();
				}

				// 3. Delete Button
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
				if (ImGui::Button("X")) {
					m_activeGraph.variables.erase(m_activeGraph.variables.begin() + i);
					ImGui::PopStyleColor();
					ImGui::PopID();
					break;
				}
				ImGui::PopStyleColor();

				// 4. Default Value Setter
				ImGui::SetNextItemWidth(208.0f);
				if (var.type == PinType::Float) { ImGui::DragFloat("##DefFlt", &var.floatValue, 0.1f); }
				else if (var.type == PinType::Int) { ImGui::DragInt("##DefInt", &var.intValue); }
				else if (var.type == PinType::Vector3) { ImGui::DragFloat3("##DefVec3", var.vec3Value.data(), 0.1f); }
				else if (var.type == PinType::Bool) { ImGui::Checkbox("Default Value", &var.boolValue); }

				ImGui::Dummy(ImVec2(0, 5));
				ImGui::PopID();
			}
			ImGui::EndChild();
			ImGui::SameLine();



			// ----- Node Editor Canvas -----
			ed::SetCurrentEditor(m_editorContext);
			ImVec2 editorSize = ImGui::GetContentRegionAvail();
			ImGui::SetItemAllowOverlap();
			ed::Begin("My Node Editor", editorSize);

			for (auto& node : m_activeGraph.nodes) {
				ed::BeginNode(node.id);

				// -- Header Text --
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

				std::string displayTitle = node.name;
				if (node.type == NodeType::Math_Add || node.type == NodeType::Math_Subtract ||
					node.type == NodeType::Math_Multiply || node.type == NodeType::Math_Divide) {
					displayTitle += " (" + VisualScriptSerializer::pinTypeToString(node.operationType) + ")";
				}

				ImGui::TextUnformatted(displayTitle.c_str());
				ImGui::Dummy(ImVec2(0, 4));
				ImGui::PopStyleColor();

				float headerHeight = ImGui::GetTextLineHeight() + 4.0f + ed::GetStyle().NodePadding.y;
				float headerWidth = ImGui::CalcTextSize(displayTitle.c_str()).x + 30.0f;

				float maxOutputWidth = 0.0f;
				for (auto& pin : node.outputs) {
					float w = ImGui::CalcTextSize(pin.name.c_str()).x + 16.0f + ImGui::GetStyle().ItemSpacing.x;
					if (w > maxOutputWidth) maxOutputWidth = w;
				}

				// -- Body Content --
				ImGui::BeginGroup();
				for (auto& pin : node.inputs) {
					ed::BeginPin(pin.id, ed::PinKind::Input);

					ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
					ed::PinPivotSize(ImVec2(0, 0));

					drawPinIcon(ImVec2(16, 16), isPinLinked(pin.id), pin.type, getPinColor(pin.type));
					ImGui::SameLine();
					ImGui::Text("%s", pin.name.c_str());

					ed::EndPin();

					if (pin.type != PinType::Flow && !isPinLinked(pin.id)) {
						ImGui::SameLine();
						ImGui::PushID(pin.id);
						ImGui::PushItemWidth(110.0f);

						switch (pin.type) {
							case PinType::Float: { ImGui::DragFloat("##flt", &pin.floatValue, 0.1f); break; }
							case PinType::Int: { ImGui::DragInt("##int", &pin.intValue); break; }
							case PinType::Vector3: { ImGui::DragFloat3("##flt3", pin.vec3Value.data(), 0.1f); break; }
							case PinType::Bool: { ImGui::Checkbox("##bl", &pin.boolValue); break; }
							case PinType::String: {
								bool isVarNode = (node.type >= NodeType::Variable_GetFloat && node.type <= NodeType::Variable_SetVector3);
								bool isNamePin = isVarNode && (pin.name == "Name");

								bool pushedColor = false;

								// -- Validate --
								if (isNamePin && !pin.stringValue.empty()) {
									bool found = false;
									bool typeMatch = false;

									for (const auto& var : m_activeGraph.variables) {
										if (var.name == pin.stringValue) {
											found = true;

											PinType expectedType = PinType::None;
											if (node.type == NodeType::Variable_GetFloat || node.type == NodeType::Variable_SetFloat) expectedType = PinType::Float;
											else if (node.type == NodeType::Variable_GetInt || node.type == NodeType::Variable_SetInt) expectedType = PinType::Int;
											else if (node.type == NodeType::Variable_GetVector3 || node.type == NodeType::Variable_SetVector3) expectedType = PinType::Vector3;
											else if (node.type == NodeType::Variable_GetBool || node.type == NodeType::Variable_SetBool) expectedType = PinType::Bool;

											if (var.type == expectedType) typeMatch = true;
											break;
										}
									}

									// -- Apply the Syntax Highlighting --
									if (!found) {
										// -- Does not Exist --
										ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
										pushedColor = true;
									}
									else if (!typeMatch) {
										// -- Exists but Wrong Type --
										ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.1f, 1.0f));
										pushedColor = true;
									}
									else {
										// -- Correct Variable --
										ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
										pushedColor = true;
									}
								}

								ImGui::InputText("##str", &pin.stringValue);

								if (pushedColor) {
									ImGui::PopStyleColor();
								}
								break;
							}
							case PinType::Key: { ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[ %s ]", pin.stringValue.c_str()); break; }
							case PinType::MouseButton: { ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[ %s ]", pin.stringValue.c_str()); break; }

							default: break;
						}

						ImGui::PopItemWidth();
						ImGui::PopID();
					}
				}
				ImGui::EndGroup();

				float inputsWidth = ImGui::GetItemRectSize().x;

				float requiredGap = headerWidth - inputsWidth - maxOutputWidth;
				float gap = std::max(30.0f, requiredGap);

				ImGui::SameLine();
				ImGui::Dummy(ImVec2(gap, 0.0f));
				ImGui::SameLine();
				ImGui::BeginGroup();
				for (auto& pin : node.outputs) {
					float pinWidth = ImGui::CalcTextSize(pin.name.c_str()).x + 16.0f + ImGui::GetStyle().ItemSpacing.x;
					float offset = maxOutputWidth - pinWidth;

					if (offset > 0.0f) {
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
					}

					ed::BeginPin(pin.id, ed::PinKind::Output);
					ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
					ed::PinPivotSize(ImVec2(0, 0));

					ImGui::Text("%s", pin.name.c_str());
					ImGui::SameLine();
					drawPinIcon(ImVec2(16, 16), isPinLinked(pin.id), pin.type, getPinColor(pin.type));

					ed::EndPin();
				}
				ImGui::EndGroup();

				ed::EndNode();

				// -- Paint the Colored Background Header --
				ImDrawList* drawList = ed::GetNodeBackgroundDrawList(node.id);
				ImVec2 nodePos = ed::GetNodePosition(node.id);
				ImVec2 nodeSize = ed::GetNodeSize(node.id);

				float borderWidth = ed::GetStyle().NodeBorderWidth;

				ImVec2 headerMin = ImVec2(nodePos.x + borderWidth, nodePos.y + borderWidth);
				ImVec2 headerMax = ImVec2(nodePos.x + nodeSize.x - borderWidth, nodePos.y + headerHeight);
				ImColor headerColor = getNodeTypeColor(node.type);

				Ref<Texture2D> headerTex = EditorResourceManager::getIcon("vsp_header_bg");
				if (headerTex) {
					void* texID = GraphicsContext::get()->getImGuiTextureID(headerTex);
					ImTextureID imTexID = (ImTextureID)texID;

					drawList->AddImageRounded(
						imTexID,
						headerMin, headerMax,
						ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
						headerColor,
						ed::GetStyle().NodeRounding - 1.0f,
						ImDrawFlags_RoundCornersTop
					);
				}
				else {
					drawList->AddRectFilled(
						headerMin, headerMax, headerColor,
						ed::GetStyle().NodeRounding - 1.0f, ImDrawFlags_RoundCornersTop
					);
				}

				drawList->AddLine(
					ImVec2(headerMin.x, nodePos.y + headerHeight),
					ImVec2(headerMax.x, nodePos.y + headerHeight),
					ImColor(255, 255, 255, 60), 1.0f
				);

			}

			// -- Color the Links --
			for (auto& link : m_activeGraph.links) {
				ImColor linkColor = ImColor(255, 255, 255);
				float thickness = 2.0f;

				for (const auto& node : m_activeGraph.nodes) {
					for (const auto& pin : node.outputs) {
						if (pin.id == link.startPinID) {
							linkColor = getPinColor(pin.type);
							if (pin.type == PinType::Flow) thickness = 3.5f;
							break;
						}
					}
				}

				ed::Link(link.id, link.startPinID, link.endPinID, linkColor, thickness);
			}

			if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f)) {

				auto showLabel = [](const char* label, ImColor color) {
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
					auto size = ImGui::CalcTextSize(label);
					auto padding = ImGui::GetStyle().FramePadding;
					auto spacing = ImGui::GetStyle().ItemSpacing;
					ImVec2 cursorPos = ImGui::GetCursorPos();
					ImGui::SetCursorPos(ImVec2(cursorPos.x + spacing.x, cursorPos.y - spacing.y));
					ImVec2 screenPos = ImGui::GetCursorScreenPos();
					ImVec2 rectMin = ImVec2(screenPos.x - padding.x, screenPos.y - padding.y);
					ImVec2 rectMax = ImVec2(screenPos.x + size.x + padding.x, screenPos.y + size.y + padding.y);
					auto drawList = ImGui::GetWindowDrawList();
					drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
					ImGui::TextUnformatted(label);
				};

				ed::PinId startPinId, endPinId;
				if (ed::QueryNewLink(&startPinId, &endPinId)) {

					auto getPin = [&](int id) -> const Pin* {
						for (const auto& node : m_activeGraph.nodes) {
							for (const auto& pin : node.inputs) if (pin.id == id) return &pin;
							for (const auto& pin : node.outputs) if (pin.id == id) return &pin;
						}
						return nullptr;
					};

					const Pin* startPin = getPin((int)startPinId.Get());
					const Pin* endPin = getPin((int)endPinId.Get());

					if (startPin && endPin) {
						if (startPin == endPin) {
							ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
						}
						else if (startPin->kind == endPin->kind) {
							showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
						}
						else if (startPin->nodeID == endPin->nodeID) {
							showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
						}
						else if (startPin->type != endPin->type) {
							showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
							ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
						}
						else {
							showLabel("+ Create Link", ImColor(32, 45, 32, 180));

							if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f)) {

								int inputPinID = (startPin->kind == PinKind::Input) ? startPin->id : endPin->id;
								int outputPinID = (startPin->kind == PinKind::Output) ? startPin->id : endPin->id;

								m_activeGraph.links.erase(
									std::remove_if(m_activeGraph.links.begin(), m_activeGraph.links.end(),
										[&](const Link& link) {
											return link.endPinID == inputPinID;
										}
									), m_activeGraph.links.end()
								);

								Link newLink;
								newLink.id = m_nextLinkId++;
								newLink.startPinID = outputPinID;
								newLink.endPinID = inputPinID;
								m_activeGraph.links.push_back(newLink);
							}
						}
					}
				}
			}
			ed::EndCreate();



			// ----- Deleting -----
			if (ed::BeginDelete()) {
				// -- Delete Nodes --
				ed::NodeId deletedNodeId;
				while (ed::QueryDeletedNode(&deletedNodeId)) {
					if (ed::AcceptDeletedItem()) {
						auto nodeIt = std::find_if(m_activeGraph.nodes.begin(), m_activeGraph.nodes.end(),
							[&](const Node& node) {
								return node.id == deletedNodeId.Get(); 
							}
						);

						if (nodeIt != m_activeGraph.nodes.end()) {
							// -- Remove Links connected to the Node --
							m_activeGraph.links.erase(
								std::remove_if(m_activeGraph.links.begin(), m_activeGraph.links.end(),
									[&](const Link& link) {
										for (const auto& pin : nodeIt->inputs) if (link.endPinID == pin.id) return true;
										for (const auto& pin : nodeIt->outputs) if (link.startPinID == pin.id) return true;
										return false;
								}),
								m_activeGraph.links.end()
							);

							// -- Delete the Node itself --
							m_activeGraph.nodes.erase(nodeIt);
						}
					}
				}

				// -- Delete Links --
				ed::LinkId deletedLinkId;
				while (ed::QueryDeletedLink(&deletedLinkId)) {
					if (ed::AcceptDeletedItem()) {
						m_activeGraph.links.erase(
							std::remove_if(m_activeGraph.links.begin(), m_activeGraph.links.end(), [&](const Link& link) {
								return link.id == deletedLinkId.Get();
							}), m_activeGraph.links.end()
						);
					}
				}
			}
			ed::EndDelete();



			// ----- Right Click Menus -----
			ed::Suspend();

			static ed::NodeId contextNodeId = 0;

			if (ed::ShowNodeContextMenu(&contextNodeId)) {
				ImGui::OpenPopup("NodeOptionsPopup");
				m_keySearchString.clear();
				m_mouseSearchString.clear();
			}

			if (ed::ShowBackgroundContextMenu()) {
				ImGui::OpenPopup("NodeContextMenu");
				m_newNodePosition = ImGui::GetMousePos();
				m_nodeSearchString.clear();
			}
			else if (ed::ShowBackgroundContextMenu()) {
				ImGui::OpenPopup("BackgroundContextMenu");
				m_newNodePosition = ImGui::GetMousePos();
			}

			// -- Background Click Popup --
			if (ImGui::BeginPopup("NodeContextMenu")) {
				ImVec2 canvasPos = ed::ScreenToCanvas(m_newNodePosition);

				if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
				ImGui::InputTextWithHint("##NodeSearch", "Search nodes...", &m_nodeSearchString);
				ImGui::Separator();

				std::string searchQ = m_nodeSearchString;
				std::transform(searchQ.begin(), searchQ.end(), searchQ.begin(), ::tolower);
				bool isSearching = !searchQ.empty();

				auto beginCategory = [&](const char* name) -> bool {
					if (isSearching) return true;
					return ImGui::BeginMenu(name);
				};

				auto endCategory = [&]() {
					if (!isSearching) ImGui::EndMenu();
				};

				auto drawNode = [&](const char* name, NodeType type) {
					std::string lowerName = name;
					std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

					if (!isSearching || lowerName.find(searchQ) != std::string::npos) {
						bool isEventNode = (type >= NodeType::Event_OnCreate && type <= NodeType::Event_OnCollisionExit);
						bool canSpawn = isEventNode ? !hasNodeOfType(type) : true;
						if (ImGui::MenuItem(name, nullptr, false, canSpawn)) {
							spawnNode(type, canvasPos);
							ImGui::CloseCurrentPopup();
						}
					}
				};

				// -- EVENTS --
				if (beginCategory("Events")) {
					drawNode("On Create", NodeType::Event_OnCreate);
					drawNode("On Destroy", NodeType::Event_OnDestroy);
					drawNode("On Update", NodeType::Event_OnUpdate);
					drawNode("On Collision Enter", NodeType::Event_OnCollisionEnter);
					drawNode("On Collision Exit", NodeType::Event_OnCollisionExit);
					endCategory();
				}

				// -- ENTITY --
				if (beginCategory("Entity")) {
					drawNode("Instantiate Entity", NodeType::Entity_Instantiate);
					drawNode("Instantiate Prefab", NodeType::Entity_InstantiatePrefab);
					drawNode("Destroy Entity", NodeType::Entity_Destroy);
					drawNode("Find Entity by Name", NodeType::Entity_FindByName);
					drawNode("Emit Particles", NodeType::Entity_EmitParticles);
					endCategory();
				}

				// -- TRANSFORM --
				if (beginCategory("Transform")) {
					drawNode("Get Position", NodeType::Transform_GetPosition);
					drawNode("Set Position", NodeType::Transform_SetPosition);
					drawNode("Get Rotation", NodeType::Transform_GetRotation);
					drawNode("Set Rotation", NodeType::Transform_SetRotation);
					drawNode("Get Scale", NodeType::Transform_GetScale);
					drawNode("Set Scale", NodeType::Transform_SetScale);
					ImGui::Separator();
					drawNode("Get Forward", NodeType::Transform_GetForward);
					drawNode("Get Right", NodeType::Transform_GetRight);
					drawNode("Get Up", NodeType::Transform_GetUp);
					endCategory();
				}

				// -- AUDIO --
				if (beginCategory("Audio")) {
					drawNode("Play Audio", NodeType::Audio_Play);
					drawNode("Stop Audio", NodeType::Audio_Stop);
					drawNode("Get Volume", NodeType::Audio_GetVolume);
					drawNode("Set Volume", NodeType::Audio_SetVolume);
					endCategory();
				}

				// -- PHYSICS --
				if (beginCategory("Physics")) {
					drawNode("Add Force", NodeType::RigidBody_AddForce);
					drawNode("Add Torque", NodeType::RigidBody_AddTorque);
					drawNode("Add Impulse", NodeType::RigidBody_AddImpulse);
					drawNode("Add Radial Impulse", NodeType::RigidBody_AddRadialImpulse);
					ImGui::Separator();
					drawNode("Get Linear Velocity", NodeType::RigidBody_GetLinearVelocity);
					drawNode("Set Linear Velocity", NodeType::RigidBody_SetLinearVelocity);
					drawNode("Get Angular Velocity", NodeType::RigidBody_GetAngularVelocity);
					drawNode("Set Angular Velocity", NodeType::RigidBody_SetAngularVelocity);
					drawNode("Get Mass", NodeType::RigidBody_GetMass);
					drawNode("Set Mass", NodeType::RigidBody_SetMass);
					endCategory();
				}

				// -- INPUT --
				if (beginCategory("Input")) {
					drawNode("Is Key Pressed", NodeType::Input_IsKeyPressed);
					drawNode("Is Mouse Button Pressed", NodeType::Input_IsMouseButtonPressed);
					endCategory();
				}

				// -- LOGIC --
				if (beginCategory("Logic")) {
					drawNode("Branch (If)", NodeType::Logic_Branch);
					drawNode("Sequence)", NodeType::Logic_Sequence);
					drawNode("And (&&)", NodeType::Logic_And);
					drawNode("Or (||)", NodeType::Logic_Or);
					endCategory();
				}

				// -- MATH --
				if (beginCategory("Math")) {
					drawNode("Add (+)", NodeType::Math_Add);
					drawNode("Subtract (-)", NodeType::Math_Subtract);
					drawNode("Multiply (*)", NodeType::Math_Multiply);
					drawNode("Divide (/)", NodeType::Math_Divide);
					ImGui::Separator();
					drawNode("Equal (==)", NodeType::Math_Equal);
					drawNode("Greater (>)", NodeType::Math_Greater);
					drawNode("Less (<)", NodeType::Math_Less);
					ImGui::Separator();
					drawNode("Make Vector3", NodeType::Math_MakeVector3);
					drawNode("Break Vector3", NodeType::Math_BreakVector3);
					endCategory();
				}

				// -- VARIABLES --
				if (beginCategory("Variables")) {
					drawNode("Get Int", NodeType::Variable_GetInt);
					drawNode("Set Int", NodeType::Variable_SetInt);
					drawNode("Get Float", NodeType::Variable_GetFloat);
					drawNode("Set Float", NodeType::Variable_SetFloat);
					drawNode("Get Bool", NodeType::Variable_GetBool);
					drawNode("Set Bool", NodeType::Variable_SetBool);
					drawNode("Get Vector3", NodeType::Variable_GetVector3);
					drawNode("Set Vector3", NodeType::Variable_SetVector3);
					endCategory();
				}

				ImGui::EndPopup();
			}

			// -- Node Options Popup --
			if (ImGui::BeginPopup("NodeOptionsPopup")) {

				auto nodeIt = std::find_if(m_activeGraph.nodes.begin(), m_activeGraph.nodes.end(),
					[&](const Node& n) { return n.id == contextNodeId.Get(); }
				);

				if (nodeIt->type == NodeType::Math_Add || nodeIt->type == NodeType::Math_Subtract ||
					nodeIt->type == NodeType::Math_Multiply || nodeIt->type == NodeType::Math_Divide) {

					if (ImGui::BeginMenu("Change Data Type...")) {

						auto changeType = [&](PinType newType, PinType secondaryType = PinType::None) {
							if (nodeIt->operationType != newType) {
								nodeIt->operationType = newType;

								nodeIt->inputs[0].type = newType;
								nodeIt->inputs[1].type = (secondaryType == PinType::None) ? newType : secondaryType;
								nodeIt->outputs[0].type = newType;

								m_activeGraph.links.erase(
									std::remove_if(m_activeGraph.links.begin(), m_activeGraph.links.end(),
										[&](const Link& link) {
											for (const auto& pin : nodeIt->inputs) if (link.endPinID == pin.id) return true;
											for (const auto& pin : nodeIt->outputs) if (link.startPinID == pin.id) return true;
											return false;
										}
									), m_activeGraph.links.end()
								);
							}
						};

						if (ImGui::MenuItem("Float", nullptr, nodeIt->operationType == PinType::Float)) changeType(PinType::Float);
						if (ImGui::MenuItem("Int", nullptr, nodeIt->operationType == PinType::Int)) changeType(PinType::Int);
						if (ImGui::MenuItem("Vector3", nullptr, nodeIt->operationType == PinType::Vector3)) changeType(PinType::Vector3);

						// -- Multiply Vector3 and Float
						if (nodeIt->type == NodeType::Math_Multiply) {
							ImGui::Separator();
							if (ImGui::MenuItem("Vector3 * Float")) changeType(PinType::Vector3, PinType::Float);
						}

						ImGui::EndMenu();
					}
					ImGui::Separator();
				}

				if (nodeIt != m_activeGraph.nodes.end()) {
					ImGui::TextUnformatted(nodeIt->name.c_str());
					ImGui::Separator();

					bool hasConfigurablePins = false;
					for (auto& pin : nodeIt->inputs) {
						if (pin.type == PinType::Key) {
							hasConfigurablePins = true;
							ImGui::PushItemWidth(140.0f);

							if (ImGui::BeginCombo("Key", pin.stringValue.c_str())) {

								if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
								ImGui::InputTextWithHint("##KeySearch", "Search...", &m_keySearchString);
								ImGui::Separator();

								std::string searchQ = m_keySearchString;
								std::transform(searchQ.begin(), searchQ.end(), searchQ.begin(), ::tolower);

								for (int i = 0; i < IM_ARRAYSIZE(s_KeyNames); i++) {
									std::string lowerKey = s_KeyNames[i];
									std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

									if (searchQ.empty() || lowerKey.find(searchQ) != std::string::npos) {
										bool isSelected = (pin.stringValue == s_KeyNames[i]);
										if (ImGui::Selectable(s_KeyNames[i], isSelected)) {
											pin.stringValue = s_KeyNames[i];
											ImGui::CloseCurrentPopup();
										}

										if (isSelected && searchQ.empty()) ImGui::SetItemDefaultFocus();
									}
								}
								ImGui::EndCombo();
							}
							ImGui::PopItemWidth();
						}
						else if (pin.type == PinType::MouseButton) {
							hasConfigurablePins = true;
							ImGui::PushItemWidth(140.0f);

							if (ImGui::BeginCombo("Mouse Button", pin.stringValue.c_str())) {

								if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
								ImGui::InputTextWithHint("##MouseSearch", "Search...", &m_mouseSearchString);
								ImGui::Separator();

								std::string searchQ = m_mouseSearchString;
								std::transform(searchQ.begin(), searchQ.end(), searchQ.begin(), ::tolower);

								for (int i = 0; i < IM_ARRAYSIZE(s_MouseButtonNames); i++) {
									std::string lowerMouse = s_MouseButtonNames[i];
									std::transform(lowerMouse.begin(), lowerMouse.end(), lowerMouse.begin(), ::tolower);

									if (searchQ.empty() || lowerMouse.find(searchQ) != std::string::npos) {
										bool isSelected = (pin.stringValue == s_MouseButtonNames[i]);
										if (ImGui::Selectable(s_MouseButtonNames[i], isSelected)) {
											pin.stringValue = s_MouseButtonNames[i];
											ImGui::CloseCurrentPopup();
										}
										if (isSelected && searchQ.empty()) ImGui::SetItemDefaultFocus();
									}
								}
								ImGui::EndCombo();
							}
							ImGui::PopItemWidth();
						}
					}

					if (hasConfigurablePins) ImGui::Separator();

					// -- Break all Links --
					if (ImGui::MenuItem("Break Links")) {
						m_activeGraph.links.erase(
							std::remove_if(m_activeGraph.links.begin(), m_activeGraph.links.end(),
								[&](const Link& link) {
									for (const auto& pin : nodeIt->inputs) if (link.endPinID == pin.id) return true;
									for (const auto& pin : nodeIt->outputs) if (link.startPinID == pin.id) return true;
									return false;
								}
							),
							m_activeGraph.links.end()
						);
					}

					ImGui::Separator();

					// -- Delete Node --
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
					if (ImGui::MenuItem("Delete")) {
						ed::DeleteNode(contextNodeId);
					}
					ImGui::PopStyleColor();
				}

				ImGui::EndPopup();
			}

			ed::Resume();

			ed::End();
			ed::SetCurrentEditor(nullptr);



			// -- Drop Target for .axvs Files --
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
					std::filesystem::path relPath = static_cast<const char*>(payload->Data);
					std::filesystem::path absPath = AssetManager::getAbsolute(relPath);

					if (absPath.extension() == ".axvs") {
						openScript(absPath);
					}
				}
				ImGui::EndDragDropTarget();
			}
		}
		else {
			ImGui::Text("No Visual Script Loaded.");
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Create or open a .axvs file from the Content Browser.");

			// -- Drop Target for .axvs Files --
			ImGui::Dummy(ImGui::GetContentRegionAvail());
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
					std::filesystem::path relPath = static_cast<const char*>(payload->Data);
					std::filesystem::path absPath = AssetManager::getAbsolute(relPath);

					if (absPath.extension() == ".axvs") {
						openScript(absPath);
					}
				}
				ImGui::EndDragDropTarget();
			}

		}

		ImGui::End();
	}

	void VisualScriptPanel::spawnNode(NodeType type, ImVec2 position) {
		Node newNode;
		newNode.id = m_nextNodeId++;
		newNode.type = type;

		switch (type) {
			// -- EVENTS --
			case NodeType::Event_OnCreate: {
				newNode.name = "On Create";
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Event_OnDestroy: {
				newNode.name = "On Destroy";
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Event_OnUpdate: {
				newNode.name = "On Update";
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Delta Time", PinKind::Output, PinType::Float });
				break;
			}
			case NodeType::Event_OnCollisionEnter: {
				newNode.name = "On Collision Enter";
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Other Entity", PinKind::Output, PinType::Entity });
				break;
			}
			case NodeType::Event_OnCollisionExit: {
				newNode.name = "On Collision Exit";
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Other Entity", PinKind::Output, PinType::Entity });
				break;
			}

			// -- ENTITY --
			case NodeType::Entity_Instantiate: {
				newNode.name = "Instantiate Entity";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Entity", PinKind::Output, PinType::Entity });
				break;
			}
			case NodeType::Entity_InstantiatePrefab: {
				newNode.name = "Instantiate Prefab";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "File Path", PinKind::Input, PinType::String });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Entity", PinKind::Output, PinType::Entity });
				break;
			}
			case NodeType::Entity_Destroy: {
				newNode.name = "Destroy Entity";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Entity_FindByName: {
				newNode.name = "Find Entity by Name";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Entity", PinKind::Output, PinType::Entity });
				break;
			}
			case NodeType::Entity_EmitParticles: {
				newNode.name = "Emit Particles";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Count", PinKind::Input, PinType::Int });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}

			// -- TRANSFORM --
			case NodeType::Transform_GetPosition: {
				newNode.name = "Get Position";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::Transform_GetRotation: {
				newNode.name = "Get Rotation";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::Transform_GetScale: {
				newNode.name = "Get Scale";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::Transform_GetForward: {
				newNode.name = "Get Forward Vector";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::Transform_GetRight: {
				newNode.name = "Get Right Vector";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::Transform_GetUp: {
				newNode.name = "Get Up Vector";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::Transform_SetPosition: {
				newNode.name = "Set Position";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Transform_SetRotation: {
				newNode.name = "Set Rotation";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Transform_SetScale: {
				newNode.name = "Set Scale";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}

			// -- RIGIDBODY --
			case NodeType::RigidBody_AddForce: {
				newNode.name = "Add Force";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Force", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::RigidBody_AddTorque: {
				newNode.name = "Add Torque";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Torque", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::RigidBody_AddImpulse: {
				newNode.name = "Add Impulse";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Force", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::RigidBody_AddRadialImpulse: {
				newNode.name = "Add Radial Impulse";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Origin", PinKind::Input, PinType::Vector3 });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Radius", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Strength", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::RigidBody_GetLinearVelocity: {
				newNode.name = "Get Linear Velocity";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Velocity", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::RigidBody_SetLinearVelocity: {
				newNode.name = "Set Linear Velocity";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Velocity", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::RigidBody_GetAngularVelocity: {
				newNode.name = "Get Angular Velocity";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Velocity", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::RigidBody_SetAngularVelocity: {
				newNode.name = "Set Angular Velocity";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Velocity", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::RigidBody_GetMass: {
				newNode.name = "Get Mass";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Mass", PinKind::Output, PinType::Float });
				break;
			}
			case NodeType::RigidBody_SetMass: {
				newNode.name = "Set Mass";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Mass", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}

			// -- INPUT --
			case NodeType::Input_IsKeyPressed: {
				newNode.name = "Is Key Pressed";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Key", PinKind::Input, PinType::Key, 0.0f, 0, Vec3::zero(), false, "Space" });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Bool });
				break;
			}
			case NodeType::Input_IsMouseButtonPressed: {
				newNode.name = "Is Mouse Button Pressed";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Button", PinKind::Input, PinType::MouseButton, 0.0f, 0, Vec3::zero(), false, "Left" });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Bool });
				break;
			}

			// -- AUDIO --
			case NodeType::Audio_Play: {
				newNode.name = "Play Audio";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Audio_Stop: {
				newNode.name = "Stop Audio";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Audio_GetVolume: {
				newNode.name = "Get Volume";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Volume", PinKind::Output, PinType::Float });
				break;
			}
			case NodeType::Audio_SetVolume: {
				newNode.name = "Set Volume";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Volume", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}

			// -- ANIMATOR --
			case NodeType::Animator_Play: {
				newNode.name = "Play Animation";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Animator_Stop: {
				newNode.name = "Stop Animation";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Animator_IsPlaying: {
				newNode.name = "Is Playing";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Target", PinKind::Input, PinType::Entity });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Bool });
				break;
			}

			// -- LOGIC --
			case NodeType::Logic_Branch: {
				newNode.name = "Branch";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Condition", PinKind::Input, PinType::Bool });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "True", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "False", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Logic_Sequence: {
				newNode.name = "Sequence";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Then 0", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Then 1", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Then 2", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Then 3", PinKind::Output, PinType::Flow });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Then 4", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Logic_And: {
				newNode.name = "And (&&)";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Condition", PinKind::Input, PinType::Bool });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Condition", PinKind::Input, PinType::Bool });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "True", PinKind::Output, PinType::Bool });
				break;
			}
			case NodeType::Logic_Or: {
				newNode.name = "Or (||)";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Condition", PinKind::Input, PinType::Bool });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Condition", PinKind::Input, PinType::Bool });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "True", PinKind::Output, PinType::Bool });
				break;
			}


			// -- MATH --
			case NodeType::Math_Add: {
				newNode.name = "Add (+)";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "A", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "B", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Float });
				break;
			}
			case NodeType::Math_Subtract: {
				newNode.name = "Subtract (-)";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "A", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "B", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Float });
				break;
			}
			case NodeType::Math_Multiply: {
				newNode.name = "Multiply (*)";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "A", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "B", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Float });
				break;
			}
			case NodeType::Math_Divide: {
				newNode.name = "Divide (/)";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "A", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "B", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Float });
				break;
			}
			case NodeType::Math_Equal: {
				newNode.name = "Equal (==)";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "A", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "B", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Bool });
				break;
			}
			case NodeType::Math_Greater: {
				newNode.name = "Greater (>)";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "A", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "B", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Bool });
				break;
			}
			case NodeType::Math_Less: {
				newNode.name = "Less (<)";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "A", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "B", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Bool });
				break;
			}
			case NodeType::Math_MakeVector3: {
				newNode.name = "Make Vector3";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "X", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Y", PinKind::Input, PinType::Float });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Z", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Vector", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::Math_BreakVector3: {
				newNode.name = "Break Vector3";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Vector", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "X", PinKind::Output, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Y", PinKind::Output, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Z", PinKind::Output, PinType::Float });
				break;
			}

			// -- VARIABLES --
			case NodeType::Variable_GetFloat: {
				newNode.name = "Get Float";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String, 0.0f, 0, Vec3::zero(), false, "NewVar" });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Output, PinType::Float });
				break;
			}
			case NodeType::Variable_SetFloat: {
				newNode.name = "Set Float";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String, 0.0f, 0, Vec3::zero(), false, "NewVar" });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Input, PinType::Float });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Variable_GetInt: {
				newNode.name = "Get Int";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String, 0.0f, 0, Vec3::zero(), false, "NewVar" });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Output, PinType::Int });
				break;
			}
			case NodeType::Variable_SetInt: {
				newNode.name = "Set Int";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String, 0.0f, 0, Vec3::zero(), false, "NewVar" });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Input, PinType::Int });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Variable_GetBool: {
				newNode.name = "Get Bool";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String, 0.0f, 0, Vec3::zero(), false, "NewVar" });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Output, PinType::Bool });
				break;
			}
			case NodeType::Variable_SetBool: {
				newNode.name = "Set Bool";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String, 0.0f, 0, Vec3::zero(), false, "NewVar" });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Input, PinType::Bool });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}
			case NodeType::Variable_GetVector3: {
				newNode.name = "Get Vector3";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String, 0.0f, 0, Vec3::zero(), false, "NewVar" });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Output, PinType::Vector3 });
				break;
			}
			case NodeType::Variable_SetVector3: {
				newNode.name = "Set Vector3";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Execute", PinKind::Input, PinType::Flow });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Name", PinKind::Input, PinType::String, 0.0f, 0, Vec3::zero(), false, "NewVar" });
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Value", PinKind::Input, PinType::Vector3 });
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Next", PinKind::Output, PinType::Flow });
				break;
			}

		}

		m_activeGraph.nodes.push_back(newNode);
		ed::SetNodePosition(newNode.id, position);
	}

	bool VisualScriptPanel::isPinLinked(int pinId) const {
		for (const auto& link : m_activeGraph.links) {
			if (link.startPinID == pinId || link.endPinID == pinId) {
				return true;
			}
		}
		return false;
	}

	bool VisualScriptPanel::hasNodeOfType(NodeType type) const {
		for (const auto& node : m_activeGraph.nodes) {
			if (node.type == type) {
				return true;
			}
		}
		return false;
	}

	void VisualScriptPanel::drawToolbar() {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(4, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::BeginChild("VSToolbar", ImVec2(0, 32), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
		ImGui::SetCursorPosX(8.0f);

		if (ImGui::Button("Compile & Save", ImVec2(120, 24))) {
			compileAndSave();
		}

		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 150.0f);
		std::string displayFile = m_currentFilePath.empty() ? "Unsaved" : m_currentFilePath.filename().string();
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", displayFile.c_str());

		ImGui::EndChild();

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
	}

	void VisualScriptPanel::compileAndSave() {

		if (!ProjectManager::hasProject()) {
			AX_CORE_LOG_WARN("Cannot save Visual Script: No active project!");
			return;
		}

		if (m_currentFilePath.empty()) {
			AX_CORE_LOG_WARN("No .axvs file path associated with this graph!");
			return;
		}

		if (m_activeGraph.className.empty()) {
			m_activeGraph.className = m_currentFilePath.stem().string();
		}

		std::filesystem::path scriptDir = ProjectManager::getProject()->getAssetsPath() / "scripts";
		if (!std::filesystem::exists(scriptDir)) {
			std::filesystem::create_directories(scriptDir);
		}

		m_activeGraph.className = m_currentFilePath.stem().string();

		VisualScriptSerializer::serialize(m_activeGraph, m_currentFilePath);

		std::string generatedCS = VisualScriptCompiler::compileGraph(m_activeGraph);
		std::filesystem::path csPath = ProjectManager::getProject()->getProjectPath() / "Scripts" / (m_activeGraph.className + ".cs");

		std::ofstream out(csPath);
		if (out.is_open()) {
			out << generatedCS;
			out.close();
			AX_CORE_LOG_INFO("Successfully generated C# script: {}", csPath.filename().string());
		}
		else {
			AX_CORE_LOG_ERROR("Failed to write C# script to {}", csPath.string());
		}

		ProjectManager::triggerScriptAssemblyLoad();
	}

	ImColor VisualScriptPanel::getNodeTypeColor(NodeType type) {
		switch (type) {
			// -- EVENT --
			case NodeType::Event_OnCreate:
			case NodeType::Event_OnDestroy:
			case NodeType::Event_OnUpdate:
			case NodeType::Event_OnCollisionEnter:
			case NodeType::Event_OnCollisionExit:		return ImColor(160, 20, 20); // Dark Red

			// -- ENTITY --
			case NodeType::Entity_Instantiate:
			case NodeType::Entity_InstantiatePrefab:
			case NodeType::Entity_Destroy:
			case NodeType::Entity_FindByName:
			case NodeType::Entity_EmitParticles:		return ImColor(40, 90, 200); // Blue

			// -- TRANSFORM --
			case NodeType::Transform_GetPosition:
			case NodeType::Transform_SetPosition:
			case NodeType::Transform_GetRotation:
			case NodeType::Transform_SetRotation:
			case NodeType::Transform_GetScale:
			case NodeType::Transform_SetScale:
			case NodeType::Transform_GetForward:
			case NodeType::Transform_GetRight:
			case NodeType::Transform_GetUp:				return ImColor(40, 130, 40); // Green

			// -- AUDIO --
			case NodeType::Audio_Play:
			case NodeType::Audio_Stop:
			case NodeType::Audio_GetVolume:
			case NodeType::Audio_SetVolume:				return ImColor(200, 100, 20); // Orange

			// -- PHYSICS --
			case NodeType::RigidBody_AddForce:
			case NodeType::RigidBody_AddTorque:
			case NodeType::RigidBody_AddImpulse:
			case NodeType::RigidBody_AddRadialImpulse:
			case NodeType::RigidBody_GetLinearVelocity:
			case NodeType::RigidBody_SetLinearVelocity:
			case NodeType::RigidBody_GetAngularVelocity:
			case NodeType::RigidBody_SetAngularVelocity:
			case NodeType::RigidBody_GetMass:
			case NodeType::RigidBody_SetMass:			return ImColor(20, 50, 140); // Dark Navy Blue

			// -- INPUT --
			case NodeType::Input_IsKeyPressed:
			case NodeType::Input_IsMouseButtonPressed:	return ImColor(120, 20, 160); // Purple

			// -- ANIMATOR --
			case NodeType::Animator_Play:
			case NodeType::Animator_Stop:
			case NodeType::Animator_IsPlaying:			return ImColor(20, 120, 120); // Teal

			// -- LOGIC --
			case NodeType::Logic_Branch:
			case NodeType::Logic_Sequence:
			case NodeType::Logic_And:
			case NodeType::Logic_Or:					return ImColor(100, 100, 100); // Silver

			// -- MATH --
			case NodeType::Math_Add:
			case NodeType::Math_Subtract:
			case NodeType::Math_Multiply:
			case NodeType::Math_Divide:
			case NodeType::Math_Equal:
			case NodeType::Math_Greater:
			case NodeType::Math_Less:
			case NodeType::Math_MakeVector3:
			case NodeType::Math_BreakVector3:			return ImColor(60, 100, 60); // Olive

			// -- VARIABLES --
			case NodeType::Variable_GetFloat:
			case NodeType::Variable_SetFloat:
			case NodeType::Variable_GetInt:
			case NodeType::Variable_SetInt:
			case NodeType::Variable_GetBool:
			case NodeType::Variable_SetBool:
			case NodeType::Variable_GetVector3:
			case NodeType::Variable_SetVector3:			return ImColor(100, 10, 10); // Dark Red

			default: return ImColor(80, 80, 80); // Gray
		}
	}

	ImColor VisualScriptPanel::getPinColor(PinType type) {
		switch (type) {
			case PinType::Flow:			return ImColor(255, 255, 255); // White
			case PinType::Bool:			return ImColor(220, 48, 48); // Red
			case PinType::Int:			return ImColor(68, 201, 156); // Teal
			case PinType::Float:		return ImColor(147, 226, 74); // Green
			case PinType::String:		return ImColor(218, 0, 183); // Magenta
			case PinType::Vector3:		return ImColor(255, 202, 36); // Yellow
			case PinType::Entity:		return ImColor(51, 150, 215); // Cyan
			case PinType::Key:			return ImColor(200, 200, 200); // Gray
			case PinType::MouseButton:	return ImColor(200, 200, 200); // Gray
			case PinType::None:			return ImColor(200, 200, 200); // Gray
			default:					return ImColor(200, 200, 200); // Gray
		}
	}

	void VisualScriptPanel::drawPinIcon(ImVec2 size, bool connected, PinType type, ImColor color) {
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float visualOffsetY = 2.5f;

		// -- Flow Pin --
		if (type == PinType::Flow) {
			float x = pos.x;
			float y = pos.y + visualOffsetY;
			float w = size.x;
			float h = size.y;

			ImVec2 pts[5] = {
				ImVec2(x + w * 0.1f, y + h * 0.2f),
				ImVec2(x + w * 0.6f, y + h * 0.2f),
				ImVec2(x + w * 0.9f, y + h * 0.5f),
				ImVec2(x + w * 0.6f, y + h * 0.8f),
				ImVec2(x + w * 0.1f, y + h * 0.8f) 
			};

			if (connected) {
				drawList->AddConvexPolyFilled(pts, 5, color);
			}
			else {
				drawList->AddPolyline(pts, 5, color, ImDrawFlags_Closed, 2.0f);
			}
		}

		// -- Circle Pin --
		else {
			ImVec2 center(pos.x + size.x * 0.5f, pos.y + visualOffsetY + size.y * 0.5f);
			float radius = size.x * 0.4f;

			if (connected) {
				drawList->AddCircleFilled(center, radius, color);
			}
			else {
				drawList->AddCircle(center, radius, color, 0, 2.0f);
			}
		}

		ImGui::Dummy(size);
	}

}
