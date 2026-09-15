#include "studiopch.h"
#include "VisualScriptPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/Renderer.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SImage.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SHorizontalSplitBox.h>
#include <Silica/include/SCollapsingHeader.h>
#include <Silica/include/SInputFieldFloat.h>
#include <Silica/include/SInputFieldInt.h>
#include <Silica/include/SInputFieldVec3Float.h>
#include <Silica/include/SComboBox.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/SMenuAnchor.h>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/project/ProjectManager.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/scripting/VisualScriptSerializer.h"
#include "AxionStudio/Source/scripting/VisualScriptCompiler.h"
#include "AxionStudio/Source/ui/SilicaHelpers.h"
#include "AxionStudio/Source/ui/EditorTheme.h"
#include "AxionStudio/Source/ui/panels/VisualScriptNodeRegistry.h"
#include "AxionStudio/Source/core/SilicaContext.h"

namespace {
	constexpr float LEFT_PANEL_WIDTH = 260.0f;
	constexpr float EMPTY_TEXT_WIDTH = 300.0f;
	constexpr float VAR_INPUT_WIDTH = 96.0f;
	constexpr float VAR_TYPE_WIDTH = 88.0f;
	constexpr float NODE_OFFSET = 24.0f;
}

namespace Axion {

	Silica::WidgetPtr VisualScriptPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.onDragOver = [](const Silica::DragDropPayload& payload) {
					if (payload.type == "AssetPath" && std::any_cast<std::filesystem::path>(payload.data).extension() == ".axvs") return Silica::EventReply::handled();
					return Silica::EventReply::unhandled();
				},
				.onDrop = [this](const Silica::DragDropPayload& payload) mutable {
					if (payload.type == "AssetPath") {
						auto path = std::any_cast<std::filesystem::path>(payload.data);
						if (path.extension() == ".axvs") {
							EditorActionQueue::push([this, path]() mutable { openScript(path); });
							return Silica::EventReply::handled();
						}
					}
					return Silica::EventReply::unhandled();
				}
				});

			m_nodeEditor = Silica::MakeWidget<Silica::SNodeEditor>({
				.onBackgroundContextClick = [this](Silica::Vec2 pos) { return buildNodeContextMenu(pos); },
				.onNodeContextClick = [this](Silica::NodeID id, Silica::Vec2 pos) { return buildNodeSpecificContextMenu(id, pos); }
			});

			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void VisualScriptPanel::rebuildUI() {
		EditorActionQueue::push([this]() { rebuildUI_Internal(); });
	}

	void VisualScriptPanel::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		if (!ProjectManager::hasProject()) {
			m_uiRoot->setChild(SilicaHelpers::MakeEmptyState("No Project Loaded.\n\nPlease load or create a project from the top menu bar to view visual scripts.", EMPTY_TEXT_WIDTH));
			return;
		}

		if (m_currentFilePath.empty()) {
			m_uiRoot->setChild(SilicaHelpers::MakeEmptyState("No Visual Script Loaded.\n\nOpen a .axvs file from the Content Browser.", EMPTY_TEXT_WIDTH));
			return;
		}

		auto mainContent = Silica::MakeWidget<Silica::SHorizontalSplitBox>({
			.leftWidth = LEFT_PANEL_WIDTH,
			.leftContent = buildVariablesPanel(),
			.rightContent = m_nodeEditor
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = buildToolbar(),
			.contentArea = mainContent
		}));
	}

	Silica::WidgetPtr VisualScriptPanel::buildToolbar() {

		// -- Options Menu --
		auto optionsMenu = Silica::MakeWidget<Silica::SAlign>({
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = Silica::MakeWidget<Silica::SMenuAnchor>({
				.openOnHover = false,
				.openToRight = true,
				.anchorContent = Silica::MakeWidget<Silica::SButton>({
					.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
					.color = Silica::Color::transparent(),
					.hoverColor = Silica::Color(255, 255, 255, 20),
					.onClick = []() { return Silica::EventReply::unhandled(); },
					.child = Silica::MakeWidget<Silica::SImage>({
						.textureID = SilicaContext::getIcon("GearIcon"),
						.tint = Silica::GetTheme().Text_Main,
						.desiredSize = { EditorTheme::ICON_SIZE_SMALL, EditorTheme::ICON_SIZE_SMALL }
					})
				}),
				.menuContent = Silica::MakeWidget<Silica::SBox>({
					.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
					.explicitSize = Silica::Vec2{ EditorTheme::OPTIONS_MENU_WIDTH, 0.0f },
					.borderThickness = Silica::GetTheme().Border_Thickness,
					.backgroundColor = Silica::GetTheme().Background_Popup,
					.child = Silica::MakeWidget<Silica::SVerticalBox>({
						.spacing = EditorTheme::SPACING_SMALL,
						.slots = {
							{ {0,0}, SilicaHelpers::MakeOptionsMenuItem("Close Script", [this]() {
								closeActiveScript();
							}) }
						}
					})
				})
			})
		});

		auto compileBtn = SilicaHelpers::MakeToolbarBtn("Compile & Save", Silica::GetTheme().Accent_Primary, [this]() {
			compileAndSave();
		});

		std::string displayFile = m_currentFilePath.empty() ? "Unsaved" : m_currentFilePath.filename().string();
		auto fileLabel = Silica::MakeWidget<Silica::SAlign>({
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = displayFile, .color = Silica::GetTheme().Text_Dim })
		});

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::TOOLBAR_PADDING_X, 0.0f },
			.explicitSize = Silica::Vec2{ 0.0f, EditorTheme::TOOLBAR_HEIGHT },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = EditorTheme::TOOLBAR_SPACING,
				.slots = {
					{ {0,0}, optionsMenu },
					{ {0,0}, compileBtn },
					{ {1,0}, Silica::MakeWidget<Silica::SBox>({.backgroundColor = Silica::Color::transparent() }) },
					{ {0,0}, fileLabel }
				}
			})
		});
	}

	Silica::WidgetPtr VisualScriptPanel::buildVariablesPanel() {
		auto varList = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = EditorTheme::SPACING_MEDIUM });

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

		varList->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({.color = Silica::GetTheme().Border_Secondary }) });

		for (size_t i = 0; i < m_activeGraph.variables.size(); i++) {
			auto& var = m_activeGraph.variables[i];

			auto nameInput = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{VAR_INPUT_WIDTH, 0.0f},
				.child = Silica::MakeWidget<Silica::SEditableText>({
					.initialText = var.name,
					.onTextCommitted = [this, i](const std::string& val) {
						std::string oldName = m_activeGraph.variables[i].name;
						m_activeGraph.variables[i].name = val;
						for (auto& pair : m_pinMeta) {
							if (pair.second.name == "Name" && pair.second.stringValue == oldName) pair.second.stringValue = val;
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
				.explicitSize = Silica::Vec2{VAR_TYPE_WIDTH, 0.0f},
				.child = Silica::MakeWidget<Silica::SComboBox>({
					.options = { "Float", "Int", "Bool", "Vector3" }, .initialValue = getTypeString(var.type),
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
				.spacing = EditorTheme::SPACING_MEDIUM,
				.slots = {
					{ {1,0}, nameInput },
					{ {0,0}, typeDropdown },
					{ {0,0}, deleteBtn } }
			}) });
		}

		return Silica::MakeWidget<Silica::SBox>({
			.padding = {EditorTheme::PADDING_MEDIUM, EditorTheme::PADDING_MEDIUM},
			.explicitSize = Silica::Vec2{LEFT_PANEL_WIDTH, 0.0f},
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SScrollBox>({.child = varList })
		});
	}

	Silica::WidgetPtr VisualScriptPanel::buildNodeContextMenu(Silica::Vec2 mousePos) {
		auto menuBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 2.0f });
		Silica::Vec2 canvasPos = m_nodeEditor->screenToCanvas(mousePos);

		struct NodeOption { std::string name; NodeType type; };
		std::map<std::string, std::vector<NodeOption>> categories;

		for (NodeType type : VSNodeRegistry::getAllNodeTypes()) {
			const auto& def = VSNodeRegistry::getNodeDef(type);
			categories[def.category].push_back({ def.name, type });
		}

		auto rebuildMenuUI = [this, categories, menuBox, canvasPos](const std::string& query) {
			menuBox->clearSlots();
			std::string lowerQuery = query;
			std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
			bool isSearching = !lowerQuery.empty();

			for (const auto& [catName, options] : categories) {
				auto catContentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 0.0f });
				bool hasMatchInCat = false;

				for (const auto& opt : options) {
					std::string lowerName = opt.name;
					std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

					if (!isSearching || lowerName.find(lowerQuery) != std::string::npos) {
						hasMatchInCat = true;
						bool isEventNode = (opt.type >= NodeType::Event_OnCreate && opt.type <= NodeType::Event_OnCollisionExit);
						bool canSpawn = isEventNode ? !hasNodeOfType(opt.type) : true;

						auto btn = Silica::MakeWidget<Silica::SButton>({
							.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
							.enabled = canSpawn,
							.color = Silica::Color::transparent(),
							.hoverColor = Silica::GetTheme().Accent_Primary,
							.disabledColor = Silica::Color::transparent(),
							.onClick = [this, type = opt.type, canvasPos]() {
								EditorActionQueue::push([this, type, canvasPos]() {
									Silica::Renderer::closePopups();
									spawnNode(type, { canvasPos.x - NODE_OFFSET, canvasPos.y - NODE_OFFSET });
								});
								return Silica::EventReply::handled();
							},
							.child = Silica::MakeWidget<Silica::STextBlock>({
								.text = opt.name,
								.color = canSpawn ? Silica::GetTheme().Text_Main : Silica::GetTheme().Text_Dim
							}),
						});

						if (isSearching) menuBox->addSlot({ {0,0}, btn });
						else catContentBox->addSlot({ {0,0}, btn });
					}
				}

				if (!isSearching && hasMatchInCat) {
					menuBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SCollapsingHeader>({
						.title = catName,
						.initiallyOpen = false,
						.content = catContentBox
					}) });
				}
			}
		};

		rebuildMenuUI("");

		auto searchContainer = Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
			.child = Silica::MakeWidget<Silica::SEditableText>({
				.hintText = "Search nodes...",
				.onTextChanged = rebuildMenuUI
			})
		});

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::PADDING_MEDIUM, EditorTheme::PADDING_MEDIUM },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.child = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = EditorTheme::SPACING_SMALL,
				.slots = { { {0,0}, searchContainer }, { {1,0}, Silica::MakeWidget<Silica::SScrollBox>({.child = menuBox }) } }
			})
		});
	}

	void VisualScriptPanel::spawnNode(NodeType type, Silica::Vec2 position) {
		Node node;
		node.id = m_nextNodeId++;
		node.type = type;
		m_nodeTypes[node.id] = type;

		const auto& def = VSNodeRegistry::getNodeDef(type);
		node.name = def.name;

		// -- Build Inputs Data --
		for (const auto& p : def.inputs) {
			Pin pin = { m_nextPinId++, node.id, p.name, PinKind::Input, p.type };
			if (!p.defaultVal.empty()) pin.stringValue = p.defaultVal;

			if ((type == NodeType::Variable_Get || type == NodeType::Variable_Set) && p.name == "Name") {
				pin.stringValue = m_activeGraph.variables.empty() ? "" : m_activeGraph.variables[0].name;
			}
			if (type == NodeType::Variable_Set && p.name == "Value") {
				pin.type = m_activeGraph.variables.empty() ? PinType::Float : m_activeGraph.variables[0].type;
			}
			node.inputs.push_back(pin);
			m_pinMeta[pin.id] = pin;
		}

		// -- Build Outputs Data --
		for (const auto& p : def.outputs) {
			Pin pin = { m_nextPinId++, node.id, p.name, PinKind::Output, p.type };
			if (type == NodeType::Variable_Get && p.name == "Value") {
				pin.type = m_activeGraph.variables.empty() ? PinType::Float : m_activeGraph.variables[0].type;
			}
			node.outputs.push_back(pin);
			m_pinMeta[pin.id] = pin;
		}

		// -- Create Visual Node --
		Silica::GraphNode sNode;
		sNode.id = node.id;
		sNode.title = node.name;
		sNode.headerColor = VSNodeRegistry::getNodeColor(type);
		sNode.position = { position.x, position.y };

		for (auto& pin : node.inputs) {
			Silica::NodePin sPin;
			sPin.id = pin.id; sPin.name = pin.name; sPin.type = Silica::PinType::Input;
			sPin.color = VSNodeRegistry::getPinColor(pin.type);
			sPin.inlineWidget = createInlineWidgetForPin(pin);
			sNode.inputs.push_back(sPin);
		}
		for (auto& pin : node.outputs) {
			Silica::NodePin sPin;
			sPin.id = pin.id; sPin.name = pin.name; sPin.type = Silica::PinType::Output;
			sPin.color = VSNodeRegistry::getPinColor(pin.type);
			sNode.outputs.push_back(sPin);
		}

		if (m_nodeEditor) m_nodeEditor->addNode(sNode);
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
			sNode.headerColor = VSNodeRegistry::getNodeColor(node.type);
			sNode.position = m_nodePositions.count(node.id) ? m_nodePositions[node.id] : Silica::Vec2{ 0.0f, 0.0f };

			for (const auto& pin : node.inputs) {
				if (pin.id >= m_nextPinId) m_nextPinId = pin.id + 1;
				m_pinMeta[pin.id] = pin;
				Silica::NodePin sPin;
				sPin.id = pin.id; sPin.name = pin.name; sPin.type = Silica::PinType::Input;
				sPin.color = VSNodeRegistry::getPinColor(pin.type);
				sPin.inlineWidget = createInlineWidgetForPin(pin);
				sNode.inputs.push_back(sPin);
			}

			for (const auto& pin : node.outputs) {
				if (pin.id >= m_nextPinId) m_nextPinId = pin.id + 1;
				m_pinMeta[pin.id] = pin;
				Silica::NodePin sPin;
				sPin.id = pin.id; sPin.name = pin.name; sPin.type = Silica::PinType::Output;
				sPin.color = VSNodeRegistry::getPinColor(pin.type);
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

			if (m_nodeEditor && std::filesystem::exists(layoutPath)) m_nodeEditor->loadGraph(layoutPath);
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

		syncGraphState();
		m_activeGraph.className = m_currentFilePath.stem().string();

		VisualScriptSerializer::serialize(m_activeGraph, m_currentFilePath);
		m_nodeEditor->saveGraph(m_currentLayoutFilePath);

		std::string generatedCS = VisualScriptCompiler::compileGraph(m_activeGraph);
		std::filesystem::path csPath = ProjectManager::getProject()->getProjectPath() / "Scripts" / (m_activeGraph.className + ".cs");
		std::ofstream out(csPath);
		if (out.is_open()) { out << generatedCS; out.close(); }

		ProjectManager::triggerScriptAssemblyLoad();
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
					.options = varNames, .initialValue = pin.stringValue, .searchable = false,
					.onValueChanged = [this, id = pin.id](const std::string& val) {
						m_pinMeta[id].stringValue = val;
						refreshVariableNodes();
					}
				});
			}
			return Silica::MakeWidget<Silica::SEditableText>({
				.initialText = pin.stringValue,
				.onTextChanged = [this, id = pin.id](const std::string& val) {
					m_pinMeta[id].stringValue = val;
				}
			});
		}
		else if (pin.type == PinType::Key) {
			std::vector<std::string> keys = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Zero", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Space", "Enter", "Escape", "Tab", "Left", "Right", "Up", "Down", "LeftShift", "RightShift", "LeftControl", "RightControl", "LeftAlt" };
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
			auto textBlock = Silica::MakeWidget<Silica::STextBlock>({ .text = pin.boolValue ? "True" : "False", });
			return Silica::MakeWidget<Silica::SButton>({
				.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::PADDING_SMALL },
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

	bool VisualScriptPanel::hasNodeOfType(NodeType type) const {
		if (!m_nodeEditor) return false;
		for (const auto& sNode : m_nodeEditor->getNodes()) {
			auto it = m_nodeTypes.find(sNode.id);
			if (it != m_nodeTypes.end() && it->second == type) return true;
		}
		return false;
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
					Pin* namePin = nullptr;

					for (auto& pin : node.inputs) {
						if (pin.name == "Name") { targetVarName = pin.stringValue; namePin = &pin; }
					}

					bool found = false;
					PinType actualType = PinType::Float;

					for (const auto& var : m_activeGraph.variables) {
						if (var.name == targetVarName) { actualType = var.type; found = true; break; }
					}

					if (!found) {
						if (!m_activeGraph.variables.empty()) { targetVarName = m_activeGraph.variables[0].name; actualType = m_activeGraph.variables[0].type; }
						else { targetVarName = ""; actualType = PinType::Float; }

						if (namePin) { namePin->stringValue = targetVarName; m_pinMeta[namePin->id].stringValue = targetVarName; }
					}

					for (auto& pin : node.outputs) { if (pin.name == "Value") { pin.type = actualType; m_pinMeta[pin.id].type = actualType; } }
					for (auto& pin : node.inputs) { if (pin.name == "Value") { pin.type = actualType; m_pinMeta[pin.id].type = actualType; } }
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
					Pin* p1 = findPinInGraph(link.startPinID); Pin* p2 = findPinInGraph(link.endPinID);
					return (!p1 || !p2 || p1->type != p2->type);
				}), m_activeGraph.links.end());

			setContext(m_activeGraph, m_currentFilePath);
			rebuildUI_Internal();
		});
	}

	void VisualScriptPanel::syncGraphState() {
		if (!m_nodeEditor) return;

		for (const auto& sNode : m_nodeEditor->getNodes()) m_nodePositions[sNode.id] = sNode.position;

		m_activeGraph.nodes.clear();
		for (const auto& sNode : m_nodeEditor->getNodes()) {
			Node node; node.id = sNode.id; node.name = sNode.title; node.type = m_nodeTypes[sNode.id];

			for (const auto& sPin : sNode.inputs) { if (m_pinMeta.find(sPin.id) != m_pinMeta.end()) node.inputs.push_back(m_pinMeta[sPin.id]); }
			for (const auto& sPin : sNode.outputs) { if (m_pinMeta.find(sPin.id) != m_pinMeta.end()) node.outputs.push_back(m_pinMeta[sPin.id]); }
			m_activeGraph.nodes.push_back(node);
		}

		m_activeGraph.links.clear();
		for (const auto& sLink : m_nodeEditor->getLinks()) {
			Link link; link.id = sLink.id; link.startPinID = sLink.startPin; link.endPinID = sLink.endPin;
			m_activeGraph.links.push_back(link);
		}
	}

	Silica::WidgetPtr VisualScriptPanel::buildNodeSpecificContextMenu(Silica::NodeID id, Silica::Vec2 mousePos) {
		auto menuBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = EditorTheme::SPACING_SMALL });

		// -- CLONE BUTTON --
		menuBox->addSlot({ {0,0}, SilicaHelpers::MakeContextMenuItem("Clone Node", [this, id]() {
			syncGraphState();

			Node* srcNode = nullptr;
			for (auto& n : m_activeGraph.nodes) { if (n.id == id) { srcNode = &n; break; } }

			if (srcNode) {
				Node clonedNode = *srcNode;
				clonedNode.id = m_nextNodeId++;
				m_nodeTypes[clonedNode.id] = clonedNode.type;

				Silica::Vec2 srcPos = m_nodePositions[id]; m_nodePositions[clonedNode.id] = { srcPos.x + NODE_OFFSET, srcPos.y + NODE_OFFSET };

				for (auto& pin : clonedNode.inputs) { int oldId = pin.id; pin.id = m_nextPinId++; pin.nodeID = clonedNode.id; m_pinMeta[pin.id] = m_pinMeta[oldId]; m_pinMeta[pin.id].id = pin.id; m_pinMeta[pin.id].nodeID = clonedNode.id; }
				for (auto& pin : clonedNode.outputs) { int oldId = pin.id; pin.id = m_nextPinId++; pin.nodeID = clonedNode.id; m_pinMeta[pin.id] = m_pinMeta[oldId]; m_pinMeta[pin.id].id = pin.id; m_pinMeta[pin.id].nodeID = clonedNode.id; }

				m_activeGraph.nodes.push_back(clonedNode);
				setContext(m_activeGraph, m_currentFilePath);
				rebuildUI_Internal();
			}
		}) });

		// -- DELETE BUTTON --
		menuBox->addSlot({ {0,0}, SilicaHelpers::MakeContextMenuItem("Delete Node", [this, id]() {
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
				[&](const Link& link) { return std::find(nodePinIDs.begin(), nodePinIDs.end(), link.startPinID) != nodePinIDs.end() || std::find(nodePinIDs.begin(), nodePinIDs.end(), link.endPinID) != nodePinIDs.end();
			}), m_activeGraph.links.end());

			m_activeGraph.nodes.erase(std::remove_if(m_activeGraph.nodes.begin(), m_activeGraph.nodes.end(), [id](const Node& n) { return n.id == id; }), m_activeGraph.nodes.end());
			m_nodePositions.erase(id); m_nodeTypes.erase(id);

			setContext(m_activeGraph, m_currentFilePath);
			rebuildUI_Internal();
		}, Silica::GetTheme().Accent_Danger) });

		return Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::PADDING_SMALL, EditorTheme::PADDING_SMALL },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.backgroundColor = Silica::GetTheme().Background_Popup,
			.child = menuBox
		});
	}

	void VisualScriptPanel::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<ProjectChangedEvent>(AX_BIND_EVENT_FN(VisualScriptPanel::onProjectChanged));
		dispatcher.dispatch<AssetRenamedEvent>(AX_BIND_EVENT_FN(VisualScriptPanel::onAssetRenamed));
		dispatcher.dispatch<AssetDeletedEvent>(AX_BIND_EVENT_FN(VisualScriptPanel::onAssetDeleted));
	}

	EventReply VisualScriptPanel::onProjectChanged(ProjectChangedEvent& ev) {
		rebuildUI();
		return EventReply::unhandled();
	}

	EventReply VisualScriptPanel::onAssetRenamed(AssetRenamedEvent& e) {
		if (m_currentFilePath == e.getOldPath()) {
			m_currentFilePath = e.getNewPath();
			m_currentLayoutFilePath = (e.getNewPath().parent_path() / (e.getNewPath().stem().string() + "_layout.axvslayout")).string();
			m_activeGraph.className = e.getNewPath().stem().string();
			rebuildUI();
		}
		return EventReply::unhandled();
	}

	EventReply VisualScriptPanel::onAssetDeleted(AssetDeletedEvent& e) {
		if (m_currentFilePath == e.getPath()) closeActiveScript();
		return EventReply::unhandled();
	}

}
