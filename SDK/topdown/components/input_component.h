#pragma once
#include "../../../entity_system/entity_system.h"
#include <algorithm>

namespace components {
	struct input : public augmentations::entity_system::component {
		void add(unsigned action) {
			registered_events.push_back(action);
			std::sort(registered_events.begin(), registered_events.end());
		}

		void remove(unsigned action) {
			auto it = std::lower_bound(registered_events.begin(), registered_events.end(), action);

			if (it != registered_events.end() && *it == action)
				registered_events.erase(it);
		}

		bool find(unsigned action) const {
			return std::binary_search(registered_events.begin(), registered_events.end(), action);
		}

		const std::vector<unsigned>& get_events() const {
			return registered_events;
		}

	private:
		std::vector<unsigned> registered_events;
	};
}



