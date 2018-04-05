#pragma once
#include <unordered_set>
#include <vector>

#include "game/transcendental/entity_id.h"

using selection_group_type = std::unordered_set<entity_id>;

namespace augs {
	struct introspection_access;
};

class editor_selection_groups {
	using selection_groups_type = std::vector<selection_group_type>;

	friend augs::introspection_access;

	// GEN INTROSPECTOR class editor_selection_groups
	selection_groups_type groups;
	// END GEN INTROSPECTOR

	template <class C, class F>
	static bool on_group_entry_of_impl(C& self, const entity_id id, F callback) {
		for (std::size_t i = 0; i < self.groups.size(); ++i) {
			auto& g = self.groups[i];

			if (auto it = g.find(id); it != g.end()) {
				callback(i, g, it);
				return true;
			}
		}

		return false;
	}

public:
	template <class... Args>
	decltype(auto) on_group_entry_of(Args&&... args) {
		return on_group_entry_of_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_group_entry_of(Args&&... args) const {
		return on_group_entry_of_impl(*this, std::forward<Args>(args)...);
	}

	void set_group(unsigned, entity_id);
	selection_group_type& new_group();
};
