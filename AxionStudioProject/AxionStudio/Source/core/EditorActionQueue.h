#pragma once

#include <functional>
#include <vector>

namespace Axion {

	class EditorActionQueue {
	public:

		static void shutdown() {
			s_actions.clear();
		}

		static void push(std::function<void()> action) {
			s_actions.push_back(action);
		}

		static void execute() {
			if (s_actions.empty()) return;

			std::vector<std::function<void()>> currentActions;
			currentActions.swap(s_actions);

			for (auto& action : currentActions) {
				action();
			}
		}

	private:

		inline static std::vector<std::function<void()>> s_actions;

	};

}
