#pragma once
#include "application/setups/debugger/debugger_selection_groups.h"

template <class C, class F>
bool debugger_selection_groups::on_group_entry_of_impl(C& self, const selection_group_unit id, F&& callback) {
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

template <class... Args>
decltype(auto) debugger_selection_groups::on_group_entry_of(Args&&... args) {
	return on_group_entry_of_impl(*this, std::forward<Args>(args)...);
}

template <class... Args>
decltype(auto) debugger_selection_groups::on_group_entry_of(Args&&... args) const {
	return on_group_entry_of_impl(*this, std::forward<Args>(args)...);
}
template <class F>
bool debugger_selection_groups::for_each_sibling(const selection_group_unit id, F callback) const {
	return on_group_entry_of(
		id, 
		[&callback](auto, const auto& group, auto) {
			for_each_in(group.entries, callback);
		}
	);
}
