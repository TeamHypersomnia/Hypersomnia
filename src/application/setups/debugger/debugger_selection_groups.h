#pragma once
#include <cstddef>
#include <unordered_set>
#include <vector>

#include "game/cosmos/entity_id.h"

namespace augs {
	struct introspection_access;
};

struct change_group_property_command;

using selection_group_unit = entity_id;
using selection_group_entries = std::unordered_set<selection_group_unit>;

struct debugger_selection_group {
	// GEN INTROSPECTOR struct debugger_selection_group
	std::string name;
	selection_group_entries entries;
	// END GEN INTROSPECTOR
};

struct debugger_property_accessors;

class debugger_selection_groups {
	using selection_groups_type = std::vector<debugger_selection_group>;

	friend augs::introspection_access;
	friend debugger_property_accessors;

	template <class C, class F>
	static bool on_group_entry_of_impl(C& self, selection_group_unit id, F&& callback);

	// GEN INTROSPECTOR class debugger_selection_groups
	selection_groups_type groups;
	// END GEN INTROSPECTOR

public:
	const auto& get_groups() const {
		return groups;
	}

	template <class... Args>
	decltype(auto) on_group_entry_of(Args&&... args);

	template <class... Args>
	decltype(auto) on_group_entry_of(Args&&... args) const;

	template <class F>
	bool for_each_sibling(const selection_group_unit id, F callback) const;

	void set_group(selection_group_unit, unsigned group_id);
	debugger_selection_group& new_group();

	std::string get_free_group_name(const std::string& pattern) const;

	std::size_t find_group_by(const std::string& name) const;
	std::size_t get_group_by(const std::string& name);

	void clear_dead_entities(const cosmos&);
};
