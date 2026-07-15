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

namespace Axion {

	Silica::WidgetPtr VisualScriptPanel::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::Color(20, 20, 20, 255),
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
				.font = m_font,
				.onBackgroundContextClick = [this](Silica::Vec2 pos) { return buildNodeContextMenu(pos); },
				.onNodeContextClick = [](Silica::NodeID id, Silica::Vec2 pos) {  /* Implement node deletion menus here later! */ return nullptr; }
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
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "No Visual Script Loaded.\nOpen a .axvs file from the Content Browser.", .font = m_font })
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
			.color = Silica::Color(45, 45, 45, 255),
			.hoverColor = Silica::Color(70, 130, 200, 255),
			.onClick = [this]() {
				compileAndSave();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Compile & Save", .font = m_font})
		});

		std::string displayFile = m_currentFilePath.empty() ? "Unsaved" : m_currentFilePath.filename().string();
		auto fileLabel = Silica::MakeWidget<Silica::SAlign>({
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = displayFile, .color = Silica::Color(150, 150, 150, 255), .font = m_font})
		});

		auto makeSpacer = []() { return Silica::MakeWidget<Silica::SBox>({ .backgroundColor = Silica::Color::transparent() }); };

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { 8.0f, 6.0f },
			.backgroundColor = Silica::Color(30, 30, 30, 255),
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

		// -- Header --
		varList->addSlot({ {0,0}, Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {1,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "VARIABLES", .font = m_font}) },
				{ {0,0}, Silica::MakeWidget<Silica::SButton>({
					.padding = {4,2}, .color = Silica::Color(50,50,50,255),
					.onClick = [this]() {
						Variable newVar;
						newVar.name = "Var_" + std::to_string(m_activeGraph.variables.size());
						newVar.type = PinType::Float;
						m_activeGraph.variables.push_back(newVar);
						rebuildUI();
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = "+", .font = m_font})
				})}
			}
		}) });

		varList->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 2}, .backgroundColor = Silica::Color(50,50,50,255)}) });

		// -- Variables --
		for (size_t i = 0; i < m_activeGraph.variables.size(); i++) {
			auto& var = m_activeGraph.variables[i];

			auto nameInput = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{120.0f, 0.0f},
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = var.name, .font = m_font,
					.onTextChanged = [&var](const std::string& val) { var.name = val; }
				})
			});

			auto deleteBtn = Silica::MakeWidget<Silica::SButton>({
				.padding = {4,2}, .color = Silica::Color(150, 50, 50, 255),
				.onClick = [this, i]() {
					m_activeGraph.variables.erase(m_activeGraph.variables.begin() + i);
					rebuildUI();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = "X", .font = m_font})
			});

			varList->addSlot({ {0,0}, Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 5.0f,
				.slots = {
					{ {1,0}, nameInput },
					{ {0,0}, deleteBtn }
				}
			}) });
		}

		return Silica::MakeWidget<Silica::SBox>({
			.padding = {10.0f, 10.0f},
			.explicitSize = Silica::Vec2{250.0f, 0.0f},
			.backgroundColor = Silica::Color(25, 25, 25, 255),
			.child = Silica::MakeWidget<Silica::SScrollBox>({.child = varList })
		});
	}

	Silica::WidgetPtr VisualScriptPanel::buildNodeContextMenu(Silica::Vec2 mousePos) {
		auto menuBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });

		struct NodeOption { std::string name; NodeType type; std::function<Silica::WidgetPtr()> createWidget; };
		struct NodeCategory { std::string name; std::vector<NodeOption> options; };

		auto categories = std::make_shared<std::vector<NodeCategory>>();

		Silica::Vec2 canvasPos = m_nodeEditor->screenToCanvas(mousePos);

		// -- Helper Functions --
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
					.hoverColor = Silica::Color(70, 130, 200, 255),
					.disabledColor = Silica::Color::transparent(),
					.onClick = [this, type, canvasPos]() {
						spawnNode(type, { canvasPos.x - 20.0f, canvasPos.y - 20.0f });
						// TODO: Close popup logic
						return Silica::EventReply::handled();
					},
					.child = Silica::MakeWidget<Silica::STextBlock>({
						.text = label,
						.color = canSpawn ? Silica::Color(255, 255, 255, 255) : Silica::Color(100, 100, 100, 255),
						.font = m_font,
					}),
				});
			};

			if (!categories->empty()) {
				categories->back().options.push_back({ label, type, btnGenerator });
			}
		};

		// -- Menu Options --
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
		addMenuOption("Get Float", NodeType::Variable_GetFloat);
		addMenuOption("Set Float", NodeType::Variable_SetFloat);
		addMenuOption("Get Int", NodeType::Variable_GetInt);
		addMenuOption("Set Int", NodeType::Variable_SetInt);
		addMenuOption("Get Bool", NodeType::Variable_GetBool);
		addMenuOption("Set Bool", NodeType::Variable_SetBool);
		addMenuOption("Get Vector3", NodeType::Variable_GetVector3);
		addMenuOption("Set Vector3", NodeType::Variable_SetVector3);

		// -- Lambda To Rebuild UI Based On Search --
		auto rebuildMenuUI = [this, categories, menuBox](const std::string& query) {
			menuBox->clearSlots();

			std::string lowerQuery = query;
			std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

			bool isSearching = !lowerQuery.empty();

			for (const auto& cat : *categories) {
				if (isSearching) {
					// -- Search Mode --
					for (const auto& opt : cat.options) {
						std::string lowerName = opt.name;
						std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

						if (lowerName.find(lowerQuery) != std::string::npos) {
							menuBox->addSlot({ {0,0}, opt.createWidget() });
						}
					}
				}
				else {
					// -- Default Mode --
					auto catContentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });
					for (const auto& opt : cat.options) {
						catContentBox->addSlot({ {0,0}, opt.createWidget() });
					}

					auto header = Silica::MakeWidget<Silica::SCollapsingHeader>({
						.title = cat.name,
						.initiallyOpen = false,
						.font = m_font,
						.content = catContentBox
					});

					menuBox->addSlot({ {0,0}, header });
				}
			}
		};

		rebuildMenuUI("");

		auto searchBar = Silica::MakeWidget<Silica::SEditableText>({
			.hintText = "Search nodes...",
			.font = m_font,
			.onTextChanged = rebuildMenuUI
		});

		auto searchContainer = Silica::MakeWidget<Silica::SBox>({
			.padding = { 4.0f, 4.0f },
			.backgroundColor = Silica::Color(45, 45, 45, 255),
			.child = searchBar
		});

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.backgroundColor = Silica::Color(45, 45, 45, 255),
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

			// -- Build Input Pins & Inline Widgets --
			for (const auto& pin : node.inputs) {
				if (pin.id >= m_nextPinId) m_nextPinId = pin.id + 1;
				m_pinMeta[pin.id] = pin;

				Silica::WidgetPtr inlineWidget = nullptr;

				if (pin.type == PinType::Float) {
					std::string defVal = std::to_string(pin.floatValue);
					defVal.erase(defVal.find_last_not_of('0') + 1, std::string::npos);
					if (defVal.back() == '.') defVal += "0";

					inlineWidget = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = defVal,
						.font = m_font,
						.onTextChanged = [this, id = pin.id](const std::string& val) {
							try {
								m_pinMeta[id].floatValue = std::stof(val);
							}
							catch (...) {}
						}
					});
				}
				else if (pin.type == PinType::Int) {
					inlineWidget = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = std::to_string(pin.intValue),
						.font = m_font,
						.onTextChanged = [this, id = pin.id](const std::string& val) {
							try {
								m_pinMeta[id].intValue = std::stoi(val);
							}
							catch (...) {}
						}
					});
				}
				else if (pin.type == PinType::String) {
					inlineWidget = Silica::MakeWidget<Silica::SEditableText>({
						.initialText = pin.stringValue,
						.font = m_font,
						.onTextChanged = [this, id = pin.id](const std::string& val) {
							m_pinMeta[id].stringValue = val;
						}
					});
				}

				Silica::NodePin sPin;
				sPin.id = pin.id;
				sPin.name = pin.name;
				sPin.type = Silica::PinType::Input;
				sPin.color = getPinColor(pin.type);
				sPin.inlineWidget = inlineWidget;

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
			if (m_nodeEditor) m_nodeEditor->addLink(link.id, link.startPinID, link.endPinID, Silica::Color::white());
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
		if (m_nodeEditor) m_nodeEditor->clear();
		rebuildUI();
	}

	void VisualScriptPanel::compileAndSave() {
		if (!ProjectManager::hasProject() || m_currentFilePath.empty() || !m_nodeEditor) return;

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

		m_activeGraph.className = m_currentFilePath.stem().string();

		// -- Save To .axvs File --
		VisualScriptSerializer::serialize(m_activeGraph, m_currentFilePath);

		// -- Save Layout To .axvslayout --
		m_nodeEditor->saveGraph(m_currentLayoutFilePath);

		// -- Compile To C# --
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
			case NodeType::Variable_GetFloat: {
				node.name = "Get Float";
				addInput("Name", PinType::String, "NewVar");
				addOutput("Value", PinType::Float);
				break;
			}
			case NodeType::Variable_SetFloat: {
				node.name = "Set Float";
				addInput("Execute", PinType::Flow);
				addInput("Name", PinType::String, "NewVar");
				addInput("Value", PinType::Float);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Variable_GetInt: {
				node.name = "Get Int";
				addInput("Name", PinType::String, "NewVar");
				addOutput("Value", PinType::Int);
				break;
			}
			case NodeType::Variable_SetInt: {
				node.name = "Set Int";
				addInput("Execute", PinType::Flow);
				addInput("Name", PinType::String, "NewVar");
				addInput("Value", PinType::Int);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Variable_GetBool: {
				node.name = "Get Bool";
				addInput("Name", PinType::String, "NewVar");
				addOutput("Value", PinType::Bool);
				break;
			}
			case NodeType::Variable_SetBool: {
				node.name = "Set Bool";
				addInput("Execute", PinType::Flow);
				addInput("Name", PinType::String, "NewVar");
				addInput("Value", PinType::Bool);
				addOutput("Next", PinType::Flow);
				break;
			}
			case NodeType::Variable_GetVector3: {
				node.name = "Get Vector3";
				addInput("Name", PinType::String, "NewVar");
				addOutput("Value", PinType::Vector3);
				break;
			}
			case NodeType::Variable_SetVector3: {
				node.name = "Set Vector3";
				addInput("Execute", PinType::Flow);
				addInput("Name", PinType::String, "NewVar");
				addInput("Value", PinType::Vector3);
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
			Silica::WidgetPtr inlineWidget = nullptr;

			if (pin.type == PinType::Float) {

				std::string defVal = std::to_string(pin.floatValue);
				defVal.erase(defVal.find_last_not_of('0') + 1, std::string::npos);
				if (defVal.back() == '.') defVal += "0";

				inlineWidget = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = defVal,
					.font = m_font,
					.onTextChanged = [this, id = pin.id](const std::string& val) {
						try {
							m_pinMeta[id].floatValue = std::stof(val);
						}
						catch (...) {}
					}
				});
			}
			else if (pin.type == PinType::Int) {
				inlineWidget = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = std::to_string(pin.intValue),
					.font = m_font,
					.onTextChanged = [this, id = pin.id](const std::string& val) {
						try {
							m_pinMeta[id].intValue = std::stoi(val);
						}
						catch (...) {}
					}
				});
			}
			else if (pin.type == PinType::String) {
				inlineWidget = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = pin.stringValue,
					.font = m_font,
					.onTextChanged = [this, id = pin.id](const std::string& val) {
						m_pinMeta[id].stringValue = val;
					}
				});
			}

			Silica::NodePin sPin;
			sPin.id = pin.id;
			sPin.name = pin.name;
			sPin.type = Silica::PinType::Input;
			sPin.color = getPinColor(pin.type);
			sPin.inlineWidget = inlineWidget;

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
		if (type >= NodeType::Event_OnCreate && type <= NodeType::Event_OnCollisionExit) return Silica::Color(160, 20, 20, 255);
		if (type >= NodeType::Entity_Instantiate && type <= NodeType::Entity_EmitParticles) return Silica::Color(40, 90, 200, 255);
		if (type >= NodeType::Transform_GetPosition && type <= NodeType::Transform_GetUp) return Silica::Color(40, 130, 40, 255);
		return Silica::Color(80, 80, 80, 255);
	}

	Silica::Color VisualScriptPanel::getPinColor(PinType type) {
		if (type == PinType::Flow) return Silica::Color(255, 255, 255, 255);
		if (type == PinType::Float) return Silica::Color(147, 226, 74, 255);
		if (type == PinType::Vector3) return Silica::Color(255, 202, 36, 255);
		return Silica::Color(200, 200, 200, 255);
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

}
