#include "studiopch.h"
#include "NetworkPanel.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SBorderLayout.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SEditableText.h>
#include <Silica/include/SInputFieldInt.h>
#include <Silica/include/SScrollBox.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SSeparator.h>

#include "AxionNetwork/Source/NetworkServer.h"
#include "AxionNetwork/Source/NetworkClient.h"

#include "AxionEngine/Source/scene/SceneManager.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

namespace Axion {

	NetworkPanel::NetworkPanel(AXNetwork::NetworkServer* server, AXNetwork::NetworkClient* client)
		: m_server(server), m_client(client) {}

	Silica::WidgetPtr NetworkPanel::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({.borderThickness = Silica::GetTheme().Border_Thickness });
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void NetworkPanel::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void NetworkPanel::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		// -- Status text and color --
		std::string statusText = "Offline";
		Silica::Color statusColor = Silica::GetTheme().Text_Dim;

		if (m_currentState == NetworkUIState::RunningServer) {
			statusText = "Hosting Server";
			statusColor = Silica::GetTheme().Accent_Primary;
		}
		else if (m_currentState == NetworkUIState::RunningClient) {
			statusText = "Connected as Client";
			statusColor = Silica::GetTheme().Accent_Success;
		}

		// -- Top toolbar --
		auto topBarBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { 10.0f, 10.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Multiplayer: " })
					})},
					{ {1, 0}, Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({
							.text = statusText,
							.color = statusColor
						})
					})}
				}
			})
		});


		// -- Helper function --
		auto MakePropertyRow = [&](const std::string& label, Silica::WidgetPtr valueWidget) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(120.0f, 0.0f),
						.backgroundColor = Silica::Color::transparent(),
						.child = Silica::MakeWidget<Silica::SAlign>({
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = label })
						})
					})},
					{ {1, 0}, valueWidget }
				}
			});
		};

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 15.0f });

		// ----- OFFLINE UI -----
		if (m_currentState == NetworkUIState::Offline) {

			// -- Server start setup --
			auto serverControls = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {0,0}, MakePropertyRow("Host Port", Silica::MakeWidget<Silica::SInputFieldInt>({
						.initialValue = m_targetPort,
						.onValueChanged = [this](int val) { m_targetPort = val; }
					}))},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 0.0f, 6.0f },
						.onClick = [this]() {
							if (m_server->start((uint16_t)m_targetPort)) {
								m_currentState = NetworkUIState::RunningServer;
								rebuildUI();
							}
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Start Listen Server"})
						})
					})},
				}
			});
			contentBox->addSlot({ {0, 0}, serverControls });

			contentBox->addSlot({ {0, 0}, Silica::MakeWidget<Silica::SSeparator>({}) });

			// -- Client connect setup --
			auto clientControls = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 8.0f,
				.slots = {
					{ {0,0}, MakePropertyRow("Target IP", Silica::MakeWidget<Silica::SEditableText>({
						.initialText = m_targetIP,
						.onTextCommitted = [this](const std::string& val) { m_targetIP = val; }
					}))},
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 0.0f, 6.0f },
						.onClick = [this]() {
							if (m_client->connect(m_targetIP, (uint16_t)m_targetPort)) {
								m_currentState = NetworkUIState::RunningClient;
								rebuildUI();
							}
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Connect as Client"})
						})
					})}
				}
			});
			contentBox->addSlot({ {0, 0}, clientControls });
		}

		// ----- SERVER DASHBOARD UI -----
		else if (m_currentState == NetworkUIState::RunningServer) {

			auto serverDashboard = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0,0}, MakePropertyRow("Listening Port", Silica::MakeWidget<Silica::STextBlock>({.text = std::to_string(m_targetPort) }))},
					{ {0,0}, MakePropertyRow("Local IP", Silica::MakeWidget<Silica::STextBlock>({.text = "127.0.0.1 (Localhost)" }))},

					{ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) },

					// -- Debug tools --
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = "Server Tools", .color = Silica::GetTheme().Text_Dim }) },

					{ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) },

					// -- Shutdown --
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 0.0f, 6.0f },
						.color = Silica::GetTheme().Accent_Danger,
						.onClick = [this]() {
							m_server->stop();
							m_currentState = NetworkUIState::Offline;
							rebuildUI();
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Stop Server"})
						})
					})}
				}
			});
			contentBox->addSlot({ {0, 0}, serverDashboard });
		}

		// ----- CLIENT DASHBOARD UI -----
		else if (m_currentState == NetworkUIState::RunningClient) {

			auto clientDashboard = Silica::MakeWidget<Silica::SVerticalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0,0}, MakePropertyRow("Connected To", Silica::MakeWidget<Silica::STextBlock>({.text = m_targetIP + ":" + std::to_string(m_targetPort) }))},

					{ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) },

					// -- Debug tools --
					{ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
						.text = "Client Tools",
						.color = Silica::GetTheme().Text_Dim
					}) },
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 0.0f, 6.0f },
						.onClick = [this]() {
							m_client->sendString("Client sent a debug ping!");
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Send Debug Ping"})
						})
					})},

					{ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) },

					// -- Disconnect --
					{ {0,0}, Silica::MakeWidget<Silica::SButton>({
						.padding = { 0.0f, 6.0f },
						.color = Silica::GetTheme().Accent_Danger,
						.onClick = [this]() {
							m_client->disconnect();
							m_currentState = NetworkUIState::Offline;
							rebuildUI();
							return Silica::EventReply::handled();
						},
						.child = Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Disconnect"})
						})
					})}
				}
			});
			contentBox->addSlot({ {0, 0}, clientDashboard });
		}

		// -- Final Assembly --
		auto paddedContent = Silica::MakeWidget<Silica::SBox>({
			.padding = { 10.0f, 10.0f },
			.child = contentBox
		});

		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({.child = paddedContent });

		auto borderLayout = Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = topBarBox,
			.contentArea = scrollBox
		});

		m_uiRoot->setChild(borderLayout);
	}

}
