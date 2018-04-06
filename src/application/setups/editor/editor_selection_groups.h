#pragma once
#include <unordered_set>
#include <vector>

#include "game/transcendental/entity_id.h"

namespace augs {
	struct introspection_access;
};

struct change_group_property_command;

using selection_group_entries = std::unordered_set<entity_id>;

struct editor_selection_group {
	// GEN INTROSPECTOR struct editor_selection_group
	std::string name;
	selection_group_entries entries;
	// END GEN INTROSPECTOR
};

class editor_selection_groups {
	using selection_groups_type = std::vector<editor_selection_group>;

	friend augs::introspection_access;
	friend change_group_property_command;

	template <class C, class F>
	static bool on_group_entry_of_impl(C& self, const entity_id id, F callback) {
		for (std::size_t i = 0; i < self.groups.size(); ++i) {
			auto& g = self.groups[i];
			auto& entries = g.entries;

			if (auto it = entries.find(id); it != entries.end()) {
				callback(i, g, it);
				return true;
			}
		}

		return false;
	}

	// GEN INTROSPECTOR class editor_selection_groups
	selection_groups_type groups;
	// END GEN INTROSPECTOR

public:
	const auto& get_groups() const {
		return groups;
	}

	template <class... Args>
	decltype(auto) on_group_entry_of(Args&&... args) {
		return on_group_entry_of_impl(*this, std::forward<Args>(args)...);
	}

	template <class... Args>
	decltype(auto) on_group_entry_of(Args&&... args) const {
		return on_group_entry_of_impl(*this, std::forward<Args>(args)...);
	}

	void set_group(unsigned, entity_id);
	editor_selection_group& new_group();

	std::string get_free_group_name(const std::string& pattern) const;

	std::size_t find_group_by(const std::string& name) const;
	std::size_t get_group_by(const std::string& name);
};
