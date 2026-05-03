#include "VisualScriptPanel.h"

#include "AxionEngine/Vendor/imgui/imgui.h"
#include "AxionEngine/Vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/scripting/VisualScriptSerializer.h"
#include "AxionStudio/Source/scripting/VisualScriptCompiler.h"

namespace Axion {

	namespace ed = ax::NodeEditor;

	VisualScriptPanel::VisualScriptPanel(const std::string& name)
		: Panel(name) {}

	VisualScriptPanel::~VisualScriptPanel() {
		shutdown();
	}

	void VisualScriptPanel::setup() {
		m_config = {};
		m_config.SettingsFile = "AxionStudio/Config/VisualScriptSettings.json";
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

	void VisualScriptPanel::onEvent(Event& e) {}

	void VisualScriptPanel::onGuiRender() {
		ImGui::Begin("Visual Script Editor");

		if (!m_activeGraph.className.empty()) {

			drawToolbar();

			ed::SetCurrentEditor(m_editorContext);
			ImVec2 editorSize = ImGui::GetContentRegionAvail();
			ImGui::SetItemAllowOverlap();
			ed::Begin("My Node Editor", editorSize);

			for (auto& node : m_activeGraph.nodes) {
				ed::BeginNode(node.id);

				// -- Header Text --
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::TextUnformatted(node.name.c_str());
				ImGui::Dummy(ImVec2(0, 4));
				ImGui::PopStyleColor();

				float headerHeight = ImGui::GetTextLineHeight() + 4.0f + ed::GetStyle().NodePadding.y;
				float headerWidth = ImGui::CalcTextSize(node.name.c_str()).x + 30.0f;

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
						ImGui::PushItemWidth(80.0f);

						switch (pin.type) {
							case PinType::Float: { ImGui::DragFloat("##flt", &pin.floatValue, 0.1f); break; }
							case PinType::Int: { ImGui::DragInt("##int", &pin.intValue); break; }
							case PinType::Vector3: { ImGui::DragFloat3("##flt3", pin.vec3Value.data(), 0.1f); break; }
							case PinType::Bool: { ImGui::Checkbox("##bl", &pin.boolValue); break; }
							case PinType::String: { ImGui::InputText("##str", &pin.stringValue); break; }
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

				drawList->AddRectFilled(
					headerMin, headerMax, headerColor,
					ed::GetStyle().NodeRounding - 1.0f, ImDrawFlags_RoundCornersTop
				);

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

					const Pin* startPin = getPin(startPinId.Get());
					const Pin* endPin = getPin(endPinId.Get());

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
											if (startPin->type == PinType::Flow && link.startPinID == outputPinID) return true;
											return link.endPinID == inputPinID;
									}),
									m_activeGraph.links.end()
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
			}

			if (ed::ShowBackgroundContextMenu()) {
				ImGui::OpenPopup("NodeContextMenu");
				m_newNodePosition = ImGui::GetMousePos();
			}
			else if (ed::ShowBackgroundContextMenu()) {
				ImGui::OpenPopup("BackgroundContextMenu");
				m_newNodePosition = ImGui::GetMousePos();
			}

			// -- Background Click Popup --
			if (ImGui::BeginPopup("NodeContextMenu")) {
				ImVec2 canvasPos = ed::ScreenToCanvas(m_newNodePosition);

				ImGui::TextUnformatted("Add Node...");
				ImGui::Separator();

				// -- EVENT --
				if (ImGui::BeginMenu("Events")) {
					if (ImGui::MenuItem("On Create", nullptr, false, !hasNodeOfType(NodeType::Event_OnCreate))) { spawnNode(NodeType::Event_OnCreate, canvasPos); }
					if (ImGui::MenuItem("On Destroy", nullptr, false, !hasNodeOfType(NodeType::Event_OnDestroy))) { spawnNode(NodeType::Event_OnDestroy, canvasPos); }
					if (ImGui::MenuItem("On Update", nullptr, false, !hasNodeOfType(NodeType::Event_OnUpdate))) { spawnNode(NodeType::Event_OnUpdate, canvasPos); }
					if (ImGui::MenuItem("On Collision Enter", nullptr, false, !hasNodeOfType(NodeType::Event_OnCollisionEnter))) { spawnNode(NodeType::Event_OnCollisionEnter, canvasPos); }
					if (ImGui::MenuItem("On Collision Exit", nullptr, false, !hasNodeOfType(NodeType::Event_OnCollisionExit))) { spawnNode(NodeType::Event_OnCollisionExit, canvasPos); }
					ImGui::EndMenu();
				}

				// -- ENTITY --
				if (ImGui::BeginMenu("Entity")) {
					if (ImGui::MenuItem("Instantiate Entity")) spawnNode(NodeType::Entity_Instantiate, canvasPos);
					if (ImGui::MenuItem("Instantiate Prefab")) spawnNode(NodeType::Entity_InstantiatePrefab, canvasPos);
					if (ImGui::MenuItem("Destroy Entity")) spawnNode(NodeType::Entity_Destroy, canvasPos);
					if (ImGui::MenuItem("Find Entity by Name")) spawnNode(NodeType::Entity_FindByName, canvasPos);
					if (ImGui::MenuItem("Emit Particles")) spawnNode(NodeType::Entity_EmitParticles, canvasPos);
					ImGui::EndMenu();
				}

				// -- TRANSFORM --
				if (ImGui::BeginMenu("Transform")) {
					if (ImGui::MenuItem("Get Position")) spawnNode(NodeType::Transform_GetPosition, canvasPos);
					if (ImGui::MenuItem("Set Position")) spawnNode(NodeType::Transform_SetPosition, canvasPos);
					ImGui::Separator();
					if (ImGui::MenuItem("Get Rotation")) spawnNode(NodeType::Transform_GetRotation, canvasPos);
					if (ImGui::MenuItem("Set Rotation")) spawnNode(NodeType::Transform_SetRotation, canvasPos);
					ImGui::Separator();
					if (ImGui::MenuItem("Get Scale")) spawnNode(NodeType::Transform_GetScale, canvasPos);
					if (ImGui::MenuItem("Set Scale")) spawnNode(NodeType::Transform_SetScale, canvasPos);
					ImGui::Separator();
					if (ImGui::MenuItem("Get Forward")) spawnNode(NodeType::Transform_GetForward, canvasPos);
					if (ImGui::MenuItem("Get Right")) spawnNode(NodeType::Transform_GetRight, canvasPos);
					if (ImGui::MenuItem("Get Up")) spawnNode(NodeType::Transform_GetUp, canvasPos);
					ImGui::EndMenu();
				}

				// -- AUDIO --
				if (ImGui::BeginMenu("Audio")) {
					if (ImGui::MenuItem("Play Audio")) spawnNode(NodeType::Audio_Play, canvasPos);
					if (ImGui::MenuItem("Stop Audio")) spawnNode(NodeType::Audio_Stop, canvasPos);
					if (ImGui::MenuItem("Get Volume")) spawnNode(NodeType::Audio_GetVolume, canvasPos);
					if (ImGui::MenuItem("Set Volume")) spawnNode(NodeType::Audio_SetVolume, canvasPos);
					ImGui::EndMenu();
				}

				// -- PHYSICS --
				if (ImGui::BeginMenu("Physics")) {
					if (ImGui::MenuItem("Add Force")) spawnNode(NodeType::RigidBody_AddForce, canvasPos);
					if (ImGui::MenuItem("Add Torque")) spawnNode(NodeType::RigidBody_AddTorque, canvasPos);
					if (ImGui::MenuItem("Add Radial Impulse")) spawnNode(NodeType::RigidBody_AddRadialImpulse, canvasPos);
					ImGui::Separator();
					if (ImGui::MenuItem("Get Linear Velocity")) spawnNode(NodeType::RigidBody_GetLinearVelocity, canvasPos);
					if (ImGui::MenuItem("Set Linear Velocity")) spawnNode(NodeType::RigidBody_SetLinearVelocity, canvasPos);
					ImGui::EndMenu();
				}

				// -- INPUT --
				if (ImGui::BeginMenu("Input")) {
					if (ImGui::MenuItem("Is Key Pressed")) { spawnNode(NodeType::Input_IsKeyPressed, canvasPos); }
					if (ImGui::MenuItem("Is Mouse Button Pressed")) { spawnNode(NodeType::Input_IsMouseButtonPressed, canvasPos); }
					ImGui::EndMenu();
				}

				// -- LOGIC --
				if (ImGui::BeginMenu("Logic")) {
					if (ImGui::MenuItem("Branch (If)")) spawnNode(NodeType::Logic_Branch, canvasPos);
					ImGui::EndMenu();
				}

				// -- MATH --
				if (ImGui::BeginMenu("Math")) {
					if (ImGui::MenuItem("Add (+)")) spawnNode(NodeType::Math_Add, canvasPos);
					if (ImGui::MenuItem("Subtract (-)")) spawnNode(NodeType::Math_Subtract, canvasPos);
					if (ImGui::MenuItem("Multiply (*)")) spawnNode(NodeType::Math_Multiply, canvasPos);
					if (ImGui::MenuItem("Divide (/)")) spawnNode(NodeType::Math_Divide, canvasPos);
					ImGui::Separator();
					if (ImGui::MenuItem("Equal (==)")) spawnNode(NodeType::Math_Equal, canvasPos);
					if (ImGui::MenuItem("Greater (>)")) spawnNode(NodeType::Math_Greater, canvasPos);
					if (ImGui::MenuItem("Less (<)")) spawnNode(NodeType::Math_Less, canvasPos);
					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}

			// -- Node Options Popup --
			if (ImGui::BeginPopup("NodeOptionsPopup")) {
				ImGui::TextUnformatted("Node Actions");
				ImGui::Separator();

				// -- Break all Links --
				if (ImGui::MenuItem("Break Links")) {
					auto nodeIt = std::find_if(m_activeGraph.nodes.begin(), m_activeGraph.nodes.end(),
						[&](const Node& n) {
							return n.id == contextNodeId.Get();
						}
					);

					if (nodeIt != m_activeGraph.nodes.end()) {
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
				}
				ImGui::Separator();

				// -- Delete Node --
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
				if (ImGui::MenuItem("Delete")) {
					ed::DeleteNode(contextNodeId);
				}
				ImGui::PopStyleColor();

				ImGui::EndPopup();
			}

			ed::Resume();

			ed::End();
			ed::SetCurrentEditor(nullptr);
		}
		else {
			ImGui::Text("No Visual Script Loaded.");
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Create or open a .axvs file from the Content Browser.");
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
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Key", PinKind::Input, PinType::String }); // TODO: add enum dropdown here
				newNode.outputs.push_back({ m_nextPinId++, newNode.id, "Result", PinKind::Output, PinType::Bool });
				break;
			}
			case NodeType::Input_IsMouseButtonPressed: {
				newNode.name = "Is Mouse Button Pressed";
				newNode.inputs.push_back({ m_nextPinId++, newNode.id, "Button", PinKind::Input, PinType::String });
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
			case NodeType::RigidBody_AddRadialImpulse:
			case NodeType::RigidBody_GetLinearVelocity:
			case NodeType::RigidBody_SetLinearVelocity:
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
			case NodeType::Logic_Branch:				return ImColor(100, 100, 100); // Silver

			// -- MATH --
			case NodeType::Math_Add:
			case NodeType::Math_Subtract:
			case NodeType::Math_Multiply:
			case NodeType::Math_Divide:
			case NodeType::Math_Equal:
			case NodeType::Math_Greater:
			case NodeType::Math_Less:					return ImColor(60, 100, 60); // Olive

			default: return ImColor(80, 80, 80); // Gray
		}
	}

	ImColor VisualScriptPanel::getPinColor(PinType type) {
		switch (type) {
			case PinType::Flow:		return ImColor(255, 255, 255); // White
			case PinType::Bool:		return ImColor(220, 48, 48); // Red
			case PinType::Int:		return ImColor(68, 201, 156); // Teal
			case PinType::Float:	return ImColor(147, 226, 74); // Green
			case PinType::String:	return ImColor(218, 0, 183); // Magenta
			case PinType::Vector3:	return ImColor(255, 202, 36); // Yellow
			case PinType::Entity:	return ImColor(51, 150, 215); // Cyan
			case PinType::None:		return ImColor(200, 200, 200); // Gray
			default:				return ImColor(200, 200, 200); // Gray
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
