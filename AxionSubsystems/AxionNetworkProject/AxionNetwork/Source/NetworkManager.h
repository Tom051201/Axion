#pragma once

namespace Axion::AXNetwork {

	class NetworkManager {
	public:

		static void initialize();
		static void shutdown();

	private:

		static inline bool s_initialized = false;

	};

}
