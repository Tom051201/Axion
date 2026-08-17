#pragma once

#include <functional>
#include <cstdint>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <queue>

namespace Axion {

	class JobSystem {
	public:

		static void initialize();
		static void shutdown();

		static void submit(const std::function<void()>& job);

	private:

		inline static uint32_t s_numThreads = 0;
		inline static std::vector<std::thread> s_workerThreads;

		inline static std::queue<std::function<void()>> s_jobQueue;
		inline static std::mutex s_queueMutex;
		inline static std::condition_variable s_wakeCondition;

		inline static std::atomic<bool> s_isShuttingDown = false;

		static void workerThread(uint32_t threadId);

	};

}
