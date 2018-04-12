#include "augs/misc/typesafe_sprintf.h"
#include "application/setups/editor/editor_selection_groups.h"

void editor_selection_groups::set_group(const unsigned index, const entity_id id) {
	groups[index].entries.emplace(id);
}

editor_selection_group& editor_selection_groups::new_group() {
	/* First look for an empty group */

	auto make_group_name = [](auto i){
		return "Group-" + std::to_string(i);
	};

	for (std::size_t i = 0; i < groups.size(); ++i) {
		auto& g = groups[i];

		if (g.entries.empty()) {
			g.name = make_group_name(i);
			return g;
		}
	}

	/* Create new one if all are non-empty */

	groups.emplace_back();
	auto& g = groups.back();
	g.name = make_group_name(groups.size() - 1);
	return g;
}

std::string editor_selection_groups::get_free_group_name(const std::string& pattern) const {
	for (std::size_t i = 1;; ++i) {
		const auto resolved_name = typesafe_sprintf(pattern, i);

		if (find_group_by(resolved_name) == -1) {
			return resolved_name;
		}
	}

	return "";
}

std::size_t editor_selection_groups::find_group_by(const std::string& name) const {
	for (std::size_t i = 0; i < groups.size(); ++i) {
		const auto& g = groups[i];

		if (g.name == name) {
			return i;
		}
	}

	return -1;
}

std::size_t editor_selection_groups::get_group_by(const std::string& name) {
	if (const auto id = find_group_by(name); id != -1) {
		return id;
	}

	groups.emplace_back();
	auto& g = groups.back();
	g.name = name;
	return groups.size() - 1;
}
