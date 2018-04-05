#include "application/setups/editor/editor_selection_groups.h"

void editor_selection_groups::set_group(const unsigned index, const entity_id id) {
	groups[index].emplace(id);
}

selection_group_type& editor_selection_groups::new_group() {
	/* First look for an empty group */

	for (auto& g : groups) {
		if (g.empty()) {
			return g;
		}
	}

	/* Create new one if all are non-empty */

	groups.emplace_back();
	return groups.back();
}
