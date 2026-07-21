#include "VisualScriptPanel.h"

#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/scripting/VisualScriptSerializer.h"
#include "AxionStudio/Source/scripting/VisualScriptCompiler.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SEditableText.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SImage.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/SSplitBox.h"
#include "AxionStudio/Vendor/Silica/include/SCollapsingHeader.h"
#include "AxionStudio/Vendor/Silica/include/Theme.h"
#include "AxionStudio/Vendor/Silica/include/SInputFieldFloat.h"
#include "AxionStudio/Vendor/Silica/include/SInputFieldInt.h"
#include "AxionStudio/Vendor/Silica/include/SInputFieldVec3Float.h"
#include "AxionStudio/Vendor/Silica/include/SComboBox.h"
#include "AxionStudio/Vendor/Silica/include/SSeparator.h"

namespace Axion {

	Silica::WidgetPtr VisualScriptPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::GetTheme().NodeEditor_Background,
				.onDragOver = [](const Silica::DragDropPayload& payload) {
					if (payload.type == "AssetPath") {
						auto path = std::any_cast<std::filesystem::path>(payload.data);
						if (path.extension() == ".axvs") return Silica::EventReply::handled();
					}
					return Silica::EventReply::unhandled();
				},
				.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
					if (payload.type == "AssetPath") {
						auto path = std::any_cast<std::filesystem::path>(payload.data);
						if (path.extension() == ".axvs") {
							EditorActionQueue::push([this, path]() mutable {
								openScript(path);
							});
							return Silica::EventReply::handled();
						}
					}
					return Silica::EventReply::unhandled();
				}
			});

			m_nodeEditor = Silica::MakeWidget<Silica::SNodeEditor>({
				.onBackgroundContextClick = [this](Silica::Vec2 pos) {
					return buildNodeContextMenu(pos);
				},
				.onNodeContextClick = [this](Silica::NodeID id, Silica::Vec2 pos) {
					return buildNodeSpecificContextMenu(id, pos);
				}
			});

			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void VisualScriptPanel::rebuildUI() {
		EditorActionQueue::push([this]() {
			rebuildUI_Internal();
		});
	}

	void VisualScriptPanel::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		if (m_currentFilePath.empty()) {
			m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "No Visual Script Loaded.\nOpen a .axvs file from the Content Browser." })
			}));
			return;
		}

		auto mainContent = Silica::MakeWidget<Silica::SSplitBox>({
			.leftWidth = 250.0f,
			.leftContent = buildVariablesPanel(),
			.rightContent = m_nodeEditor
		});

		auto borderLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = buildToolbar(),
			.contentArea = mainContent
		});

		m_uiRoot->setChild(borderLayout);
	}

	Silica::WidgetPtr VisualScriptPanel::buildToolbar() {
		auto compileBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 10.0f, 6.0f },
			.hoverColor = Silica::GetTheme().Accent_Primary,
			.onClick = [this]() {
				compileAndSave();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Compile & Save" })
		});

		std::string displayFile = m_currentFilePath.empty() ? "Unsaved" : m_currentFilePath.filename().string();
		auto fileLabel = Silica::MakeWidget<Silica::SAlign>({
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = Silica::MakeWidget<Silica::STextBlock>({
				.text = displayFile,
				.color = Silica::GetTheme().Text_Dim
			})
		});

		auto makeSpacer = []() { return Silica::MakeWidget<Silica::SBox>({ .backgroundColor = Silica::Color::transparent() }); };

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { 8.0f, 6.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 15.0f,
				.slots = {
					{ {0,0}, compileBtn },
					{ {1,0}, makeSpacer() },
					{ {0,0}, fileLabel }
				}
			})
		});
	}

	Silica::WidgetPtr VisualScriptPanel::buildVariablesPanel() {
		auto varList = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 6.0f });

		varList->addSlot({ {0,0}, Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "VARIABLES" }) },
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {4,2},
					.onClick = [this]() {
						Variable newVar;
						newVar.name = "Var_" + std::to_string(m_activeGraph.variables.size());
						newVar.type = PinType::Float;
						m_activeGraph.variables.push_back(newVar);
						refreshVariableNodes();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "+" })
				})}
			}
		}) });

		varList->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({ .color = Silica::GetTheme().Border_Secondary })});

		for (size_t i = 0; i < m_activeGraph.variables.size(); i++) {
			auto& var = m_activeGraph.variables[i];

			auto nameInput = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{90.0f, 0.0f},
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = var.name,
					.onTextCommitted = [this, i](const std::string& val) {
						std::string oldName = m_activeGraph.variables[i].name;
						m_activeGraph.variables[i].name = val;

						for (auto& pair : m_pinMeta) {
							if (pair.second.name == "Name" && pair.second.stringValue == oldName) {
								pair.second.stringValue = val;
							}
						}
						refreshVariableNodes();
					}
				})
			});

			auto getTypeString = [](PinType t) {
				if (t == PinType::Int) return "Int";
				if (t == PinType::Bool) return "Bool";
				if (t == PinType::Vector3) return "Vector3";
				return "Float";
			};

			auto typeDropdown = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{85.0f, 0.0f},
				.child = Silica::MakeWidget<Silica::SComboBox>({
					.options = { "Float", "Int", "Bool", "Vector3" },
					.initialValue = getTypeString(var.type),
					.onValueChanged = [this, i](const std::string& val) {
						if (val == "Int") m_activeGraph.variables[i].type = PinType::Int;
						else if (val == "Bool") m_activeGraph.variables[i].type = PinType::Bool;
						else if (val == "Vector3") m_activeGraph.variables[i].type = PinType::Vector3;
						else m_activeGraph.variables[i].type = PinType::Float;
						refreshVariableNodes();
					}
				})
			});

			auto deleteBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = {4,2},
				.hoverColor = Silica::GetTheme().Accent_Danger,
				.onClick = [this, i]() {
					m_activeGraph.variables.erase(m_activeGraph.variables.begin() + i);
					refreshVariableNodes();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X" })
			});

			varList->addSlot({ {0,0}, Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 5.0f,
				.slots = {
					{ {1,0}, nameInput },
					{ {0,0}, typeDropdown },
					{ {0,0}, deleteBtn }
				}
			}) });
		}

		return Silica::MakeWidget<Silica::SBox>({
			.padding = {10.0f, 10.0f},
			.explicitSize = Silica::Vec2{250.0f, 0.0f},
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SScrollBox>({.child = varList })
		});
	}

	Silica::WidgetPtr VisualScriptPanel::buildNodeContextMenu(Silica::Vec2 mousePos) {
		auto menuBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });

		struct NodeOption { std::string name; NodeType type; std::function<Silica::WidgetPtr()> createWidget; };
		struct NodeCategory { std::string name; std::vector<NodeOption> options; };

		auto categories = std::make_shared<std::vector<NodeCategory>>();

		Silica::Vec2 canvasPos = m_nodeEditor->screenToCanvas(mousePos);

		auto beginCategory = [&](const std::string& name) {
			categories->push_back({ name, {} });
		};

		auto addMenuOption = [&](const std::string& label, NodeType type) {
			bool isEventNode = (type >= NodeType::Event_OnCreate && type <= NodeType::Event_OnCollisionExit);
			bool canSpawn = isEventNode ? !hasNodeOfType(type) : true;

			auto btnGenerator = [this, label, type, canvasPos, canSpawn]() {
				return Silica::MakeWidget<Silica::SButton>({
					.padding = { 8.0f, 4.0f },
					.enabled = canSpawn,
					.color = Silica::Color::transparent(),
					.hoverColor = Silica::GetTheme().Accent_Primary,
					.disabledColor = Silica::Color::transparent(),
					.onClick = [this, type, canvasPos]() {
						spawnNode(type, { canvasPos.x - 20.0f, canvasPos.y - 20.0f });
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({
						.text = label,
						.color = canSpawn ? Silica::GetTheme().Text_Main : Silica::GetTheme().Text_Dim
					}),
				});
			};

			if (!categories->empty()) {
				categories->back().options.push_back({ label, type, btnGenerator });
			}
		};

		beginCategory("Events");
		addMenuOption("On Create", NodeType::Event_OnCreate);
		addMenuOption("On Destroy", NodeType::Event_OnDestroy);
		addMenuOption("On Update", NodeType::Event_OnUpdate);
		addMenuOption("On Collision Enter", NodeType::Event_OnCollisionEnter);
		addMenuOption("On Collision Exit", NodeType::Event_OnCollisionExit);

		beginCategory("Entity");
		addMenuOption("Instantiate Entity", NodeType::Entity_Instantiate);
		addMenuOption("Instantiate Prefab", NodeType::Entity_InstantiatePrefab);
		addMenuOption("Destroy Entity", NodeType::Entity_Destroy);
		addMenuOption("Find By Name", NodeType::Entity_FindByName);
		addMenuOption("Emit Particles", NodeType::Entity_EmitParticles);

		beginCategory("Transform");
		addMenuOption("Get Position", NodeType::Transform_GetPosition);
		addMenuOption("Get Rotation", NodeType::Transform_GetRotation);
		addMenuOption("Get Scale", NodeType::Transform_GetScale);
		addMenuOption("Get Forward Vector", NodeType::Transform_GetForward);
		addMenuOption("Get Right Vector", NodeType::Transform_GetRight);
		addMenuOption("Get Up Vector", NodeType::Transform_GetUp);
		addMenuOption("Set Position", NodeType::Transform_SetPosition);
		addMenuOption("Set Rotation", NodeType::Transform_SetRotation);
		addMenuOption("Set Scale", NodeType::Transform_SetScale);

		beginCategory("Rigid Body");
		addMenuOption("Add Force", NodeType::RigidBody_AddForce);
		addMenuOption("Add Torque", NodeType::RigidBody_AddTorque);
		addMenuOption("Add Impulse", NodeType::RigidBody_AddImpulse);
		addMenuOption("Add Radial Impulse", NodeType::RigidBody_AddRadialImpulse);
		addMenuOption("Get Linear Velocity", NodeType::RigidBody_GetLinearVelocity);
		addMenuOption("Set Linear Velocity", NodeType::RigidBody_SetLinearVelocity);
		addMenuOption("Get Angular Velocity", NodeType::RigidBody_GetAngularVelocity);
		addMenuOption("Set Angular Velocity", NodeType::RigidBody_SetAngularVelocity);
		addMenuOption("Get Mass", NodeType::RigidBody_GetMass);
		addMenuOption("Set Mass", NodeType::RigidBody_SetMass);

		beginCategory("Input");
		addMenuOption("Is Key Pressed", NodeType::Input_IsKeyPressed);
		addMenuOption("Is Mouse Button Pressed", NodeType::Input_IsMouseButtonPressed);

		beginCategory("Audio");
		addMenuOption("Play Audio", NodeType::Audio_Play);
		addMenuOption("Stop Audio", NodeType::Audio_Stop);
		addMenuOption("Get Volume", NodeType::Audio_GetVolume);
		addMenuOption("Set Volume", NodeType::Audio_SetVolume);

		beginCategory("Animator");
		addMenuOption("Play Animation", NodeType::Animator_Play);
		addMenuOption("Stop Animation", NodeType::Animator_Stop);
		addMenuOption("Is Playing", NodeType::Animator_IsPlaying);

		beginCategory("Logic");
		addMenuOption("Branch", NodeType::Logic_Branch);
		addMenuOption("Sequence", NodeType::Logic_Sequence);
		addMenuOption("And (&&)", NodeType::Logic_And);
		addMenuOption("Or (||)", NodeType::Logic_Or);

		beginCategory("Math");
		addMenuOption("Add (+)", NodeType::Math_Add);
		addMenuOption("Subtract (-)", NodeType::Math_Subtract);
		addMenuOption("Multiply (*)", NodeType::Math_Multiply);
		addMenuOption("Divide (/)", NodeType::Math_Divide);
		addMenuOption("Equal (==)", NodeType::Math_Equal);
		addMenuOption("Greater (>)", NodeType::Math_Greater);
		addMenuOption("Less (<)", NodeType::Math_Less);
		addMenuOption("Make Vector3", NodeType::Math_MakeVector3);
		addMenuOption("Break Vector3", NodeType::Math_BreakVector3);

		beginCategory("Variables");
		addMenuOption("Get Variable", NodeType::Variable_Get);
		addMenuOption("Set Variable", NodeType::Variable_Set);

		auto rebuildMenuUI = [this, categories, menuBox](const std::string& query) {
			menuBox->clearSlots();

			std::string lowerQuery = query;
			std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

			bool isSearching = !lowerQuery.empty();

			for (const auto& cat : *categories) {
				if (isSearching) {
					for (const auto& opt : cat.options) {
						std::string lowerName = opt.name;
						std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

						if (lowerName.find(lowerQuery) != std::string::npos) {
							menuBox->addSlot({ {0,0}, opt.createWidget() });
						}
					}
				}
				else {
					auto catContentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });
					for (const auto& opt : cat.options) {
						catContentBox->addSlot({ {0,0}, opt.createWidget() });
					}

					auto header = Silica::MakeWidget<Silica::SCollapsingHeader>({
						.title = cat.name,
						.initiallyOpen = false,
						.content = catContentBox
					});

					menuBox->addSlot({ {0,0}, header });
				}
			}
		};

		rebuildMenuUI("");

		auto searchBar = Silica::MakeWidget<Silica::SEditableText>({
			.hintText = "Search nodes...",
			.onTextChanged = rebuildMenuUI
		});

		auto searchContainer = Silica::MakeWidget<Silica::SBox>({
			.padding = { 4.0f, 4.0f },
			.child = searchBar
		});

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.child = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 4.0f,
				.slots = {
					{ {0,0}, searchContainer },
					{ {1,0}, Silica::MakeWidget<Silica::SScrollBox>({.child = menuBox }) }
				}
			})
		});
	}

	void VisualScriptPanel::setContext(const VisualGraph& graph, const std::filesystem::path& filePath) {
		m_activeGraph = graph;
		m_currentFilePath = filePath;
		m_nodeTypes.clear();
		m_pinMeta.clear();
		if (m_nodeEditor) m_nodeEditor->clear();

		m_nextNodeId = 1;
		m_nextPinId = 1000;
		m_nextLinkId = 100;

		for (const auto& node : m_activeGraph.nodes) {
			if (node.id >= m_nextNodeId) m_nextNodeId = node.id + 1;
			m_nodeTypes[node.id] = node.type;

			Silica::GraphNode sNode;
			sNode.id = node.id;
			sNode.title = node.name;
			sNode.headerColor = getNodeTypeColor(node.type);
			sNode.position = { 0.0f, 0.0f };

			if (m_nodePositions.find(node.id) != m_nodePositions.end()) {
				sNode.position = m_nodePositions[node.id];
			}
			else {
				sNode.position = { 0.0f, 0.0f };
			}

			// -- Build Input Pins & Inline Widgets --
			for (const auto& pin : node.inputs) {
				if (pin.id >= m_nextPinId) m_nextPinId = pin.id + 1;
				m_pinMeta[pin.id] = pin;

				Silica::NodePin sPin;
				sPin.id = pin.id;
				sPin.name = pin.name;
				sPin.type = Silica::PinType::Input;
				sPin.color = getPinColor(pin.type);
				sPin.inlineWidget = createInlineWidgetForPin(pin);

				sNode.inputs.push_back(sPin);
			}

			// -- Build Output Pins --
			for (const auto& pin : node.outputs) {
				if (pin.id >= m_nextPinId) m_nextPinId = pin.id + 1;
				m_pinMeta[pin.id] = pin;

				Silica::NodePin sPin;
				sPin.id = pin.id;
				sPin.name = pin.name;
				sPin.type = Silica::PinType::Output;
				sPin.color = getPinColor(pin.type);

				sNode.outputs.push_back(sPin);
			}

			if (m_nodeEditor) m_nodeEditor->addNode(sNode);
		}

		for (const auto& link : m_activeGraph.links) {
			if (link.id >= m_nextLinkId) m_nextLinkId = link.id + 1;
			if (m_nodeEditor) m_nodeEditor->addLink(link.id, link.startPinID, link.endPinID);
		}
	}

	void VisualScriptPanel::openScript(const std::filesystem::path& filePath) {
		if (!m_currentFilePath.empty()) compileAndSave();

		VisualGraph loadedGraph;
		if (VisualScriptSerializer::deserialize(loadedGraph, filePath)) {

			std::filesystem::path layoutPath = filePath.parent_path() / (filePath.stem().string() + "_layout.axvslayout");
			m_currentLayoutFilePath = layoutPath.string();

			setContext(loadedGraph, filePath);

			if (m_nodeEditor && std::filesystem::exists(layoutPath)) {
				m_nodeEditor->loadGraph(layoutPath);
			}

			rebuildUI();
		}
	}

	void VisualScriptPanel::closeActiveScript() {
		m_activeGraph = VisualGraph();
		m_currentFilePath.clear();
		m_currentLayoutFilePath.clear();
		m_nodePositions.clear();
		if (m_nodeEditor) m_nodeEditor->clear();
		rebuildUI();
	}

	void VisualScriptPanel::compileAndSave() {
		if (!ProjectManager::hasProject() || m_currentFilePath.empty() || !m_nodeEditor) return;

		for (auto& node : m_activeGraph.nodes) {
			for (auto& pin : node.inputs) if (m_pinMeta.find(pin.id) != m_pinMeta.end()) pin = m_pinMeta[pin.id];
			for (auto& pin : node.outputs) if (m_pinMeta.find(pin.id) != m_pinMeta.end()) pin = m_pinMeta[pin.id];
		}

		m_activeGraph.nodes.clear();
		for (const auto& sNode : m_nodeEditor->getNodes()) {
			Node node;
			node.id = sNode.id;
			node.name = sNode.title;
			node.type = m_nodeTypes[sNode.id];

			for (const auto& sPin : sNode.inputs) {
				if (m_pinMeta.find(sPin.id) != m_pinMeta.end()) node.inputs.push_back(m_pinMeta[sPin.id]);
			}
			for (const auto& sPin : sNode.outputs) {
				if (m_pinMeta.find(sPin.id) != m_pinMeta.end()) node.outputs.push_back(m_pinMeta[sPin.id]);
			}
			m_activeGraph.nodes.push_back(node);
		}

		m_activeGraph.links.clear();
		for (const auto& sLink : m_nodeEditor->getLinks()) {
			Link link; link.id = sLink.id; link.startPinID = sLink.startPin; link.endPinID = sLink.endPin;
			m_activeGraph.links.push_back(link);
		}

		syncGraphState();

		m_activeGraph.className = m_currentFilePath.stem().string();

		VisualScriptSerializer::serialize(m_activeGraph, m_currentFilePath);
		m_nodeEditor->saveGraph(m_currentLayoutFilePath);

		std::string generatedCS = VisualScriptCompiler::compileGraph(m_activeGraph);
		std::filesystem::path csPath = ProjectManager::getProject()->getProjectPath() / "Scripts" / (m_activeGraph.className + ".cs");
		std::ofstream out(csPath);
		if (out.is_open()) {
			out << generatedCS;
			out.close();
		}

		ProjectManager::triggerScriptAssemblyLoad();
	}

	void VisualScriptPanel::spawnNode(NodeType type, Silica::Vec2 position) {
		Node node;
		node.id = m_nextNodeId++;
		node.type = type;
		m_nodeTypes[node.id] = type;

		auto addInput = [&](const std::string& name, PinType pType, const std::string& defaultString = "") {
			Pin pin = { m_nextPinId++, node.id, name, PinKind::Input, pType };
			if (!defaultString.empty()) pin.stringValue = defaultString;
			node.inputs.push_back(pin);
			m_pinMeta[pin.id] = pin;
		};

		auto addOutput = [&](const std::string& name, PinType pType) {
			Pin pin = { m_nextPinId++, node.id, name, PinKind::Output, pType };
			node.outputs.push_back(pin);
			m_pinMeta[pin.id] = pin;
		};


		switch (type) {
			// -- EVENTS --
			case NodeType::Event_OnCreate: {
				node.name = "On Create";
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Event_OnDestroy: {
				node.name = "On Destroy";
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Event_OnUpdate: {
				node.name = "On Update";
				addOutput("Next", PinType::Flow);
				addOutput("Delta Time", PinType::Float);
				break;
			}
			case NodeType::Event_OnCollisionEnter: {
				node.name = "On Collision Enter";
				addOutput("Next", PinType::Flow);
				addOutput("Other Entity", PinType::Entity);
				break;
			}
			case NodeType::Event_OnCollisionExit: {
				node.name = "On Collision Exit";
				addOutput("Next", PinType::Flow);
				addOutput("Other Entity", PinType::Entity);
				break;
			}

			// -- ENTITY --
			case NodeType::Entity_Instantiate: {
				node.name = "Instantiate Entity";
				addInput("Execute", PinType::Flow);
				addInput("Name", PinType::String);
				addOutput("Next", PinType::Flow);
				addOutput("Entity", PinType::Entity);
				break;
			}
			case NodeType::Entity_InstantiatePrefab: {
				node.name = "Instantiate Prefab";
				addInput("Execute", PinType::Flow);
				addInput("File Path", PinType::String);
				addOutput("Next", PinType::Flow);
				addOutput("Entity", PinType::Entity);
				break;
			}
			case NodeType::Entity_Destroy: {
				node.name = "Destroy Entity";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Entity_FindByName: {
				node.name = "Find Entity by Name";
				addInput("Name", PinType::String);
				addOutput("Entity", PinType::Entity);
				break;
			}
			case NodeType::Entity_EmitParticles: {
				node.name = "Emit Particles";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Count", PinType::Int);
				addOutput("Next", PinType::Flow);
				break;
			}

			// -- TRANSFORM --
			case NodeType::Transform_GetPosition: {
				node.name = "Get Position";
				addInput("Target", PinType::Entity);
				addOutput("Result", PinType::Vector3);
				break;
			}
			case NodeType::Transform_GetRotation: {
				node.name = "Get Rotation";
				addInput("Target", PinType::Entity);
				addOutput("Result", PinType::Vector3);
				break;
			}
			case NodeType::Transform_GetScale: {
				node.name = "Get Scale";
				addInput("Target", PinType::Entity);
				addOutput("Result", PinType::Vector3);
				break;
			}
			case NodeType::Transform_GetForward: {
				node.name = "Get Forward Vector";
				addInput("Target", PinType::Entity);
				addOutput("Result", PinType::Vector3);
				break;
			}
			case NodeType::Transform_GetRight: {
				node.name = "Get Right Vector";
				addInput("Target", PinType::Entity);
				addOutput("Result", PinType::Vector3);
				break;
			}
			case NodeType::Transform_GetUp: {
				node.name = "Get Up Vector";
				addInput("Target", PinType::Entity);
				addOutput("Result", PinType::Vector3);
				break;
			}
			case NodeType::Transform_SetPosition: {
				node.name = "Set Position";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Value", PinType::Vector3);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Transform_SetRotation: {
				node.name = "Set Rotation";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Value", PinType::Vector3);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Transform_SetScale: {
				node.name = "Set Scale";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Value", PinType::Vector3);
				addOutput("Next", PinType::Flow);
				break;
			}

			// -- RIGIDBODY --
			case NodeType::RigidBody_AddForce: {
				node.name = "Add Force";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Force", PinType::Vector3);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::RigidBody_AddTorque: {
				node.name = "Add Torque";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Torque", PinType::Vector3);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::RigidBody_AddImpulse: {
				node.name = "Add Impulse";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Force", PinType::Vector3);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::RigidBody_AddRadialImpulse: {
				node.name = "Add Radial Impulse";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Origin", PinType::Vector3);
				addInput("Radius", PinType::Float);
				addInput("Strength", PinType::Float);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::RigidBody_GetLinearVelocity: {
				node.name = "Get Linear Velocity";
				addInput("Target", PinType::Entity);
				addOutput("Velocity", PinType::Vector3);
				break;
			}
			case NodeType::RigidBody_SetLinearVelocity: {
				node.name = "Set Linear Velocity";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Velocity", PinType::Vector3);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::RigidBody_GetAngularVelocity: {
				node.name = "Get Angular Velocity";
				addInput("Target", PinType::Entity);
				addOutput("Velocity", PinType::Vector3);
				break;
			}
			case NodeType::RigidBody_SetAngularVelocity: {
				node.name = "Set Angular Velocity";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Velocity", PinType::Vector3);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::RigidBody_GetMass: {
				node.name = "Get Mass";
				addInput("Target", PinType::Entity);
				addOutput("Mass", PinType::Float);
				break;
			}
			case NodeType::RigidBody_SetMass: {
				node.name = "Set Mass";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Mass", PinType::Float);
				addOutput("Next", PinType::Flow);
				break;
			}

			// -- INPUT --
			case NodeType::Input_IsKeyPressed: {
				node.name = "Is Key Pressed";
				addInput("Key", PinType::Key, "Space");
				addOutput("Result", PinType::Bool);
				break;
			}
			case NodeType::Input_IsMouseButtonPressed: {
				node.name = "Is Mouse Button Pressed";
				addInput("Button", PinType::MouseButton, "Left");
				addOutput("Result", PinType::Bool);
				break;
			}

			// -- AUDIO --
			case NodeType::Audio_Play: {
				node.name = "Play Audio";
				addInput("Execute", PinType::Flow);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Audio_Stop: {
				node.name = "Stop Audio";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Audio_GetVolume: {
				node.name = "Get Volume";
				addInput("Target", PinType::Entity);
				addOutput("Volume", PinType::Float);
				break;
			}
			case NodeType::Audio_SetVolume: {
				node.name = "Set Volume";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addInput("Volume", PinType::Float);
				addOutput("Next", PinType::Flow);
				break;
			}

			// -- ANIMATOR --
			case NodeType::Animator_Play: {
				node.name = "Play Animation";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Animator_Stop: {
				node.name = "Stop Animation";
				addInput("Execute", PinType::Flow);
				addInput("Target", PinType::Entity);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Animator_IsPlaying: {
				node.name = "Is Playing";
				addInput("Target", PinType::Entity);
				addOutput("Result", PinType::Bool);
				break;
			}

			// -- LOGIC --
			case NodeType::Logic_Branch: {
				node.name = "Branch";
				addInput("Execute", PinType::Flow);
				addInput("Condition", PinType::Bool);
				addOutput("True", PinType::Flow);
				addOutput("False", PinType::Flow);
				break;
			}
			case NodeType::Logic_Sequence: {
				node.name = "Sequence";
				addInput("Execute", PinType::Flow);
				addOutput("Then 0", PinType::Flow);
				addOutput("Then 1", PinType::Flow);
				addOutput("Then 2", PinType::Flow);
				addOutput("Then 3", PinType::Flow);
				addOutput("Then 4", PinType::Flow);
				break;
			}
			case NodeType::Logic_And: {
				node.name = "And (&&)";
				addInput("Condition A", PinType::Bool);
				addInput("Condition B", PinType::Bool);
				addOutput("True", PinType::Bool);
				break;
			}
			case NodeType::Logic_Or: {
				node.name = "Or (||)";
				addInput("Condition A", PinType::Bool);
				addInput("Condition B", PinType::Bool);
				addOutput("True", PinType::Bool);
				break;
			}


			// -- MATH --
			case NodeType::Math_Add: {
				node.name = "Add (+)";
				addInput("A", PinType::Float);
				addInput("B", PinType::Float);
				addOutput("Result", PinType::Float);
				break;
			}
			case NodeType::Math_Subtract: {
				node.name = "Subtract (-)";
				addInput("A", PinType::Float);
				addInput("B", PinType::Float);
				addOutput("Result", PinType::Float);
				break;
			}
			case NodeType::Math_Multiply: {
				node.name = "Multiply (*)";
				addInput("A", PinType::Float);
				addInput("B", PinType::Float);
				addOutput("Result", PinType::Float);
				break;
			}
			case NodeType::Math_Divide: {
				node.name = "Divide (/)";
				addInput("A", PinType::Float);
				addInput("B", PinType::Float);
				addOutput("Result", PinType::Float);
				break;
			}
			case NodeType::Math_Equal: {
				node.name = "Equal (==)";
				addInput("A", PinType::Float);
				addInput("B", PinType::Float);
				addOutput("Result", PinType::Bool);
				break;
			}
			case NodeType::Math_Greater: {
				node.name = "Greater (>)";
				addInput("A", PinType::Float);
				addInput("B", PinType::Float);
				addOutput("Result", PinType::Bool);
				break;
			}
			case NodeType::Math_Less: {
				node.name = "Less (<)";
				addInput("A", PinType::Float);
				addInput("B", PinType::Float);
				addOutput("Result", PinType::Bool);
				break;
			}
			case NodeType::Math_MakeVector3: {
				node.name = "Make Vector3";
				addInput("X", PinType::Float);
				addInput("Y", PinType::Float);
				addInput("Z", PinType::Float);
				addOutput("Vector", PinType::Vector3);
				break;
			}
			case NodeType::Math_BreakVector3: {
				node.name = "Break Vector3";
				addInput("Vector", PinType::Vector3);
				addOutput("X", PinType::Float);
				addOutput("Y", PinType::Float);
				addOutput("Z", PinType::Float);
				break;
			}

			// -- VARIABLES --
			case NodeType::Variable_Get: {
				node.name = "Get Variable";
				std::string defVar = m_activeGraph.variables.empty() ? "" : m_activeGraph.variables[0].name;
				PinType defType = m_activeGraph.variables.empty() ? PinType::Float : m_activeGraph.variables[0].type;

				addInput("Name", PinType::String, defVar);
				addOutput("Value", defType);
				break;
			}
			case NodeType::Variable_Set: {
				node.name = "Set Variable";
				std::string defVar = m_activeGraph.variables.empty() ? "" : m_activeGraph.variables[0].name;
				PinType defType = m_activeGraph.variables.empty() ? PinType::Float : m_activeGraph.variables[0].type;

				addInput("Execute", PinType::Flow);
				addInput("Name", PinType::String, defVar);
				addInput("Value", defType);
				addOutput("Next", PinType::Flow);
				break;
			}

		}

		Silica::GraphNode sNode;
		sNode.id = node.id;
		sNode.title = node.name;
		sNode.headerColor = getNodeTypeColor(node.type);
		sNode.position = { position.x, position.y };

		// -- Build Input Pins & Inline Widgets --
		for (auto& pin : node.inputs) {
			Silica::NodePin sPin;
			sPin.id = pin.id;
			sPin.name = pin.name;
			sPin.type = Silica::PinType::Input;
			sPin.color = getPinColor(pin.type);
			sPin.inlineWidget = createInlineWidgetForPin(pin);

			sNode.inputs.push_back(sPin);
		}

		// -- Build Output Pins --
		for (auto& pin : node.outputs) {
			Silica::NodePin sPin;
			sPin.id = pin.id;
			sPin.name = pin.name;
			sPin.type = Silica::PinType::Output;
			sPin.color = getPinColor(pin.type);

			sNode.outputs.push_back(sPin);
		}

		if (m_nodeEditor) m_nodeEditor->addNode(sNode);
	}

	Silica::Color VisualScriptPanel::getNodeTypeColor(NodeType type) {
		if (type >= NodeType::Event_OnCreate && type <= NodeType::Event_OnCollisionExit) return Silica::Color(140, 21, 21);
		if (type >= NodeType::Entity_Instantiate && type <= NodeType::Entity_EmitParticles) return Silica::Color(30, 136, 229);
		if (type >= NodeType::Transform_GetPosition && type <= NodeType::Transform_GetUp) return Silica::Color(230, 81, 0);
		if (type >= NodeType::RigidBody_AddForce && type <= NodeType::RigidBody_SetMass) return Silica::Color(67, 160, 71);
		if (type >= NodeType::Input_IsKeyPressed && type <= NodeType::Input_IsMouseButtonPressed) return Silica::Color(123, 31, 162);
		if (type >= NodeType::Audio_Play && type <= NodeType::Audio_SetVolume) return Silica::Color(121, 85, 72);
		if (type >= NodeType::Animator_Play && type <= NodeType::Animator_IsPlaying) return Silica::Color(0, 172, 193);
		if (type >= NodeType::Logic_Branch && type <= NodeType::Logic_Or) return Silica::Color(96, 125, 139);
		if (type >= NodeType::Math_Add && type <= NodeType::Math_BreakVector3) return Silica::Color(85, 139, 47);

		if (type == NodeType::Variable_Get || type == NodeType::Variable_Set) return Silica::Color(139, 195, 74);

		return Silica::Color(80, 80, 80);
	}

	Silica::Color VisualScriptPanel::getPinColor(PinType type) {
		if (type == PinType::Flow) return Silica::Color(255, 255, 255);
		if (type == PinType::Bool) return Silica::Color(128, 0, 0);
		if (type == PinType::Int) return Silica::Color(0, 191, 165);
		if (type == PinType::Float) return Silica::Color(139, 195, 74);
		if (type == PinType::String) return Silica::Color(233, 30, 99);
		if (type == PinType::Vector3) return Silica::Color(251, 192, 45);
		if (type == PinType::Entity) return Silica::Color(100, 181, 246);
		if (type == PinType::Key) return Silica::Color(156, 39, 176);
		if (type == PinType::MouseButton) return Silica::Color(156, 39, 176);

		return Silica::Color(200, 200, 200);
	}

	bool VisualScriptPanel::hasNodeOfType(NodeType type) const {
		if (!m_nodeEditor) return false;

		for (const auto& sNode : m_nodeEditor->getNodes()) {
			auto it = m_nodeTypes.find(sNode.id);
			if (it != m_nodeTypes.end() && it->second == type) {
				return true;
			}
		}

		return false;
	}

	void VisualScriptPanel::onAssetRenamed(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) {
		if (m_currentFilePath == oldPath) {
			m_currentFilePath = newPath;

			std::filesystem::path layoutPath = newPath.parent_path() / (newPath.stem().string() + "_layout.axvslayout");
			m_currentLayoutFilePath = layoutPath.string();

			m_activeGraph.className = newPath.stem().string();
			rebuildUI();
		}
	}

	void VisualScriptPanel::onAssetDeleted(const std::filesystem::path& path) {
		if (m_currentFilePath == path) {
			closeActiveScript();
		}
	}

	Silica::WidgetPtr VisualScriptPanel::createInlineWidgetForPin(const Pin& pin) {
		if (pin.type == PinType::Float) {
			return Silica::MakeWidget<Silica::SInputFieldFloat>({
				.initialValue = pin.floatValue,
				.onValueChanged = [this, id = pin.id](float val) {
					m_pinMeta[id].floatValue = val;
				}
			});
		}
		else if (pin.type == PinType::Int) {
			return Silica::MakeWidget<Silica::SInputFieldInt>({
				.initialValue = pin.intValue,
				.onValueChanged = [this, id = pin.id](int val) {
					m_pinMeta[id].intValue = val;
				}
			});
		}
		else if (pin.type == PinType::String) {
			NodeType nType = m_nodeTypes[pin.nodeID];
			if ((nType == NodeType::Variable_Get || nType == NodeType::Variable_Set) && pin.name == "Name") {
				std::vector<std::string> varNames;
				for (const auto& v : m_activeGraph.variables) varNames.push_back(v.name);

				return Silica::MakeWidget<Silica::SComboBox>({
					.options = varNames,
					.initialValue = pin.stringValue,
					.searchable = false,
					.onValueChanged = [this, id = pin.id](const std::string& val) {
						m_pinMeta[id].stringValue = val;
						refreshVariableNodes();
					}
				});
			}

			return Silica::MakeWidget<Silica::SEditableText>({
				.initialText = pin.stringValue,
				.onTextChanged = [this, id = pin.id](const std::string& val) { m_pinMeta[id].stringValue = val; }
			});
		}
		else if (pin.type == PinType::Key) {
			std::vector<std::string> keys = {
				"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
				"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
				"Zero", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine",
				"Space", "Enter", "Escape", "Tab", "Left", "Right", "Up", "Down",
				"LeftShift", "RightShift", "LeftControl", "RightControl", "LeftAlt"
			};
			return Silica::MakeWidget<Silica::SComboBox>({
				.options = keys,
				.initialValue = pin.stringValue.empty() ? "Space" : pin.stringValue,
				.searchable = true,
				.onValueChanged = [this, id = pin.id](const std::string& val) {
					m_pinMeta[id].stringValue = val;
				}
			});
		}
		else if (pin.type == PinType::MouseButton) {
			std::vector<std::string> btns = { "Left", "Right", "Middle", "X1", "X2" };
			return Silica::MakeWidget<Silica::SComboBox>({
				.options = btns,
				.initialValue = pin.stringValue.empty() ? "Left" : pin.stringValue,
				.onValueChanged = [this, id = pin.id](const std::string& val) {
					m_pinMeta[id].stringValue = val;
				}
			});
		}
		else if (pin.type == PinType::Bool) {
			auto textBlock = Silica::MakeWidget<Silica::STextBlock>({
				.text = pin.boolValue ? "True" : "False",
			});

			return Silica::MakeWidget<Silica::SButton>({
				.padding = { 8.0f, 2.0f },
				.onClick = [this, id = pin.id, textBlock]() {
					m_pinMeta[id].boolValue = !m_pinMeta[id].boolValue;
					textBlock->setText(m_pinMeta[id].boolValue ? "True" : "False");
					return Silica::EventReply::handled();
				},
				.child = textBlock
			});
		}
		else if (pin.type == PinType::Vector3) {
			return Silica::MakeWidget<Silica::SInputFieldVec3Float>({
				.initialValue = Silica::Vec3(pin.vec3Value.x, pin.vec3Value.y, pin.vec3Value.z),
				.labelWidth = 0.0f,
				.onValueChanged = [this, id = pin.id](Silica::Vec3 val) {
					m_pinMeta[id].vec3Value = {val.x, val.y, val.z};
				}
			});
		}

		return nullptr;
	}

	void VisualScriptPanel::refreshVariableNodes() {
		syncGraphState();

		EditorActionQueue::push([this]() {

			for (auto& node : m_activeGraph.nodes) {
				for (auto& pin : node.inputs) if (m_pinMeta.find(pin.id) != m_pinMeta.end()) pin = m_pinMeta[pin.id];
				for (auto& pin : node.outputs) if (m_pinMeta.find(pin.id) != m_pinMeta.end()) pin = m_pinMeta[pin.id];
			}

			for (auto& node : m_activeGraph.nodes) {
				if (node.type == NodeType::Variable_Get || node.type == NodeType::Variable_Set) {

					std::string targetVarName = "";
					for (auto& pin : node.inputs) {
						if (pin.name == "Name") targetVarName = pin.stringValue;
					}

					PinType actualType = PinType::Float;
					for (const auto& var : m_activeGraph.variables) {
						if (var.name == targetVarName) {
							actualType = var.type;
							break;
						}
					}

					for (auto& pin : node.outputs) {
						if (pin.name == "Value") { pin.type = actualType; m_pinMeta[pin.id].type = actualType; }
					}
					for (auto& pin : node.inputs) {
						if (pin.name == "Value") { pin.type = actualType; m_pinMeta[pin.id].type = actualType; }
					}
				}
			}

			auto findPinInGraph = [&](int pinID) -> Pin* {
				for (auto& node : m_activeGraph.nodes) {
					for (auto& p : node.inputs) if (p.id == pinID) return &p;
					for (auto& p : node.outputs) if (p.id == pinID) return &p;
				}
				return nullptr;
			};

			m_activeGraph.links.erase(std::remove_if(m_activeGraph.links.begin(), m_activeGraph.links.end(),
				[&](const Link& link) {
					Pin* p1 = findPinInGraph(link.startPinID);
					Pin* p2 = findPinInGraph(link.endPinID);

					if (!p1 || !p2 || p1->type != p2->type) {
						return true;
					}
					return false;
				}), m_activeGraph.links.end());

			setContext(m_activeGraph, m_currentFilePath);
			rebuildUI_Internal();
		});
	}

	void VisualScriptPanel::syncGraphState() {
		if (!m_nodeEditor) return;

		for (const auto& sNode : m_nodeEditor->getNodes()) {
			m_nodePositions[sNode.id] = sNode.position;
		}

		m_activeGraph.nodes.clear();
		for (const auto& sNode : m_nodeEditor->getNodes()) {
			Node node;
			node.id = sNode.id;
			node.name = sNode.title;
			node.type = m_nodeTypes[sNode.id];

			for (const auto& sPin : sNode.inputs) {
				if (m_pinMeta.find(sPin.id) != m_pinMeta.end()) node.inputs.push_back(m_pinMeta[sPin.id]);
			}
			for (const auto& sPin : sNode.outputs) {
				if (m_pinMeta.find(sPin.id) != m_pinMeta.end()) node.outputs.push_back(m_pinMeta[sPin.id]);
			}
			m_activeGraph.nodes.push_back(node);
		}

		m_activeGraph.links.clear();
		for (const auto& sLink : m_nodeEditor->getLinks()) {
			Link link; link.id = sLink.id; link.startPinID = sLink.startPin; link.endPinID = sLink.endPin;
			m_activeGraph.links.push_back(link);
		}
	}

	Silica::WidgetPtr VisualScriptPanel::buildNodeSpecificContextMenu(Silica::NodeID id, Silica::Vec2 mousePos) {
		auto menuBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });

		// -- CLONE BUTTON --
		menuBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = { 8.0f, 6.0f },
			.color = Silica::Color::transparent(),
			.onClick = [this, id]() {
				EditorActionQueue::push([this, id]() {
					syncGraphState();

					// -- Find Node To Clone --
					Node* srcNode = nullptr;
					for (auto& n : m_activeGraph.nodes) {
						if (n.id == id) { srcNode = &n; break; }
					}

					// -- Clone It And Assign Fresh IDs --
					if (srcNode) {
						Node clonedNode = *srcNode;
						clonedNode.id = m_nextNodeId++;
						m_nodeTypes[clonedNode.id] = clonedNode.type;

						Silica::Vec2 srcPos = m_nodePositions[id];
						m_nodePositions[clonedNode.id] = { srcPos.x + 20.0f, srcPos.y + 20.0f };

						for (auto& pin : clonedNode.inputs) {
							int oldId = pin.id;
							pin.id = m_nextPinId++;
							pin.nodeID = clonedNode.id;
							m_pinMeta[pin.id] = m_pinMeta[oldId];
							m_pinMeta[pin.id].id = pin.id;
							m_pinMeta[pin.id].nodeID = clonedNode.id;
						}

						for (auto& pin : clonedNode.outputs) {
							int oldId = pin.id;
							pin.id = m_nextPinId++;
							pin.nodeID = clonedNode.id;
							m_pinMeta[pin.id] = m_pinMeta[oldId];
							m_pinMeta[pin.id].id = pin.id;
							m_pinMeta[pin.id].nodeID = clonedNode.id;
						}

						m_activeGraph.nodes.push_back(clonedNode);
						setContext(m_activeGraph, m_currentFilePath);
						rebuildUI_Internal();
					}
				});
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Clone Node" })
		}) });

		// -- DELETE BUTTON --
		menuBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
			.padding = { 8.0f, 6.0f },
			.color = Silica::GetTheme().Accent_Danger,
			.onClick = [this, id]() {
				EditorActionQueue::push([this, id]() {
					syncGraphState();

					std::vector<int> nodePinIDs;
					for (const auto& n : m_activeGraph.nodes) {
						if (n.id == id) {
							for (const auto& p : n.inputs) nodePinIDs.push_back(p.id);
							for (const auto& p : n.outputs) nodePinIDs.push_back(p.id);
							break;
						}
					}

					m_activeGraph.links.erase(std::remove_if(m_activeGraph.links.begin(), m_activeGraph.links.end(),
						[&](const Link& link) {
							return std::find(nodePinIDs.begin(), nodePinIDs.end(), link.startPinID) != nodePinIDs.end() ||
								   std::find(nodePinIDs.begin(), nodePinIDs.end(), link.endPinID) != nodePinIDs.end();
						}), m_activeGraph.links.end());

					// -- Delete Node Itself --
					m_activeGraph.nodes.erase(std::remove_if(m_activeGraph.nodes.begin(), m_activeGraph.nodes.end(), [id](const Node& n) {
						return n.id == id; }
					), m_activeGraph.nodes.end());

					// -- Cleanup Maps --
					m_nodePositions.erase(id);
					m_nodeTypes.erase(id);

					setContext(m_activeGraph, m_currentFilePath);
					rebuildUI_Internal();
				});
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Delete Node" })
		}) });

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { 2.0f, 2.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.child = menuBox
		});
	}

}
