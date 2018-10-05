#pragma once
#include <unordered_set>
#include <vector>

#include "game/cosmos/entity_id.h"

namespace augs {
	struct introspection_access;
};

struct change_group_property_command;

using selection_group_unit = entity_id;
using selection_group_entries = std::unordered_set<selection_group_unit>;

struct editor_selection_group {
	// GEN INTROSPECTOR struct editor_selection_group
	std::string name;
	selection_group_entries entries;
	// END GEN INTROSPECTOR
};

struct editor_property_accessors;

class editor_selection_groups {
	using selection_groups_type = std::vector<editor_selection_group>;

	friend augs::introspection_access;
	friend editor_property_accessors;

	template <class C, class F>
	static bool on_group_entry_of_impl(C& self, const selection_group_unit id, F callback) {
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

	template <class F>
	bool for_each_sibling(const selection_group_unit id, F callback) const {
		return on_group_entry_of(
			id, 
			[&callback](auto, const auto& group, auto) {
				for_each_in(group.entries, callback);
			}
		);
	}

	void set_group(selection_group_unit, unsigned group_id);
	editor_selection_group& new_group();

	std::string get_free_group_name(const std::string& pattern) const;

	std::size_t find_group_by(const std::string& name) const;
	std::size_t get_group_by(const std::string& name);
};
