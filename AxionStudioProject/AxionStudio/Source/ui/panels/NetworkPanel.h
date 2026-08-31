#pragma once

#include <memory>
#include <string>
#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/core/Core.h"

namespace Silica {
	class SBox;
}

namespace Axion::AXNetwork {
	class NetworkServer;
	class NetworkClient;
}

namespace Axion {

	enum class NetworkUIState {
		Offline,
		RunningServer,
		RunningClient
	};

	class NetworkPanel {
	public:

		NetworkPanel(AXNetwork::NetworkServer* server, AXNetwork::NetworkClient* client);
		~NetworkPanel() = default;

		Silica::WidgetPtr getWidget();
		void rebuildUI();

	private:

		AXNetwork::NetworkServer* m_server;
		AXNetwork::NetworkClient* m_client;

		std::shared_ptr<Silica::SBox> m_uiRoot;
		bool m_rebuildQueued = false;

		NetworkUIState m_currentState = NetworkUIState::Offline;

		std::string m_targetIP = "127.0.0.1";
		int m_targetPort = 27015;

		void rebuildUI_Internal();

	};

}
