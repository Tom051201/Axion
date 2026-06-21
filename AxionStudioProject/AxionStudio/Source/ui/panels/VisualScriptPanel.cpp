#include "VisualScriptPanel.h"

#include "AxionEngine/Source/project/ProjectManager.h"
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

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	Silica::WidgetPtr VisualScriptPanel::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({ .backgroundColor = Silica::Color(20, 20, 20, 255) });

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

		auto addMenuOption = [&](const std::string& label, NodeType type) {
			menuBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SButton>({
				.padding = { 8.0f, 4.0f },
				.color = Silica::Color::transparent(),
				.hoverColor = Silica::Color(70, 130, 200, 255),
				.onClick = [this, type, mousePos]() {
					spawnNode(type, { mousePos.x - 20.0f, mousePos.y - 20.0f });
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = label, .font = m_font })
			}) });
		};

		// -- Menu Options --
		addMenuOption("Event On Update", NodeType::Event_OnUpdate);
		addMenuOption("Get Position", NodeType::Transform_GetPosition);
		addMenuOption("Set Position", NodeType::Transform_SetPosition);
		addMenuOption("Add Float (+)", NodeType::Math_Add);
		addMenuOption("Branch (If)", NodeType::Logic_Branch);

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { 5.0f, 5.0f },
			.backgroundColor = Silica::Color(45, 45, 45, 255),
			.child = Silica::MakeWidget<Silica::SScrollBox>({.child = menuBox })
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

			for (const auto& pin : node.inputs) {
				if (pin.id >= m_nextPinId) m_nextPinId = pin.id + 1;
				m_pinMeta[pin.id] = pin;
				sNode.inputs.push_back({ pin.id, pin.name, Silica::PinType::Input, getPinColor(pin.type) });
			}
			for (const auto& pin : node.outputs) {
				if (pin.id >= m_nextPinId) m_nextPinId = pin.id + 1;
				m_pinMeta[pin.id] = pin;
				sNode.outputs.push_back({ pin.id, pin.name, Silica::PinType::Output, getPinColor(pin.type) });
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

		auto addInput = [&](const std::string& name, PinType pType) {
			Pin pin = { m_nextPinId++, node.id, name, PinKind::Input, pType };
			node.inputs.push_back(pin);
			m_pinMeta[pin.id] = pin;
		};

		auto addOutput = [&](const std::string& name, PinType pType) {
			Pin pin = { m_nextPinId++, node.id, name, PinKind::Output, pType };
			node.outputs.push_back(pin);
			m_pinMeta[pin.id] = pin;
		};


		if (type == NodeType::Event_OnUpdate) {
			node.name = "On Update";
			addOutput("Next", PinType::Flow);
			addOutput("Delta Time", PinType::Float);
		}
		else if (type == NodeType::Transform_GetPosition) {
			node.name = "Get Position";
			addInput("Target", PinType::Entity);
			addOutput("Result", PinType::Vector3);
		}
		else if (type == NodeType::Transform_SetPosition) {
			node.name = "Set Position";
			addInput("Execute", PinType::Flow);
			addInput("Target", PinType::Entity);
			addInput("Value", PinType::Vector3);
			addOutput("Next", PinType::Flow);
		}

		Silica::GraphNode sNode;
		sNode.id = node.id;
		sNode.title = node.name;
		sNode.headerColor = getNodeTypeColor(node.type);
		sNode.position = { position.x, position.y };

		for (auto& pin : node.inputs) sNode.inputs.push_back({ pin.id, pin.name, Silica::PinType::Input, getPinColor(pin.type) });
		for (auto& pin : node.outputs) sNode.outputs.push_back({ pin.id, pin.name, Silica::PinType::Output, getPinColor(pin.type) });

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
}
