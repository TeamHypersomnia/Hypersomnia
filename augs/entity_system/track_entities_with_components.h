#pragma once
#include <vector>

namespace augs {
	template <class entity_id_type, class... tracked_components>
	class track_entities_with_specified_components {
	public:
		std::vector<entity_id_type> targets;

		void acquire_new_targets() {

		}
	};
}