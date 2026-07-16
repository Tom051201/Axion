#include "axpch.h"
#include "JobSystem.h"

#include "AxionEngine/Source/core/Logging.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

namespace Axion {

	void JobSystem::initialize() {
		// -- Setup Main Thread --
		PlatformUtils::setCurrentThreadName("Main Engine Thread");
		PlatformUtils::setThreadPriority(ThreadPriority::Highest);

		uint32_t hardwareThreads = std::thread::hardware_concurrency();
		s_numThreads = hardwareThreads > 2 ? hardwareThreads - 3 : 1;
		s_isShuttingDown = false;

		s_workerThreads.reserve(s_numThreads);
		for (uint32_t i = 0; i < s_numThreads; ++i) {
			s_workerThreads.emplace_back(&JobSystem::workerThread, i);
		}

		AX_CORE_LOG_INFO("JobSystem Initialized with {} worker threads.", s_numThreads);
	}

	void JobSystem::shutdown() {
		{
			// -- Lock Queue And Set Shutdown Flag --
			std::lock_guard<std::mutex> lock(s_queueMutex);
			s_isShuttingDown = true;
		}

		s_wakeCondition.notify_all();

		// -- Wait For All Threads To Finish And Safely Terminate --
		for (std::thread& worker : s_workerThreads) {
			if (worker.joinable()) {
				worker.join();
			}
		}

		s_workerThreads.clear();
		AX_CORE_LOG_INFO("JobSystem Shutdown complete.");
	}

	void JobSystem::submit(const std::function<void()>& job) {
		{
			std::lock_guard<std::mutex> lock(s_queueMutex);
			s_jobQueue.push(job);
		}

		// -- Wake Up One Sleeping Thread --
		s_wakeCondition.notify_one();
	}

	void JobSystem::workerThread(uint32_t threadId) {
		// -- Setup Worker Thread --
		std::string threadName = "Worker " + std::to_string(threadId);
		PlatformUtils::setCurrentThreadName(threadName);
		PlatformUtils::setThreadPriority(ThreadPriority::BelowNormal);

		while (true) {
			std::function<void()> currentJob;

			{
				std::unique_lock<std::mutex> lock(s_queueMutex);
				s_wakeCondition.wait(lock, [] {
					return !s_jobQueue.empty() || s_isShuttingDown;
				});

				if (s_isShuttingDown && s_jobQueue.empty()) {
					return;
				}

				currentJob = s_jobQueue.front();
				s_jobQueue.pop();
			}

			if (currentJob) {
				currentJob();
			}
		}
	}

}
