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
