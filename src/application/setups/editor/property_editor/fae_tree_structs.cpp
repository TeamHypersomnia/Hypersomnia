#include "game/transcendental/cosmos.h"

#include "application/setups/editor/property_editor/fae_tree_structs.h"

bool fae_tree_filter::any() const {
	return close_type_id || close_flavour_id || only_type_id || only_flavour_id;
}

void fae_tree_filter::perform(
	const cosmos& cosm,
	std::unordered_set<entity_id>& selections
) const {
	if (!any()) {
		return;
	}

	erase_if(
		selections,
		[&](const entity_id id) {
			if (close_type_id) {
				return id.type_id == *close_type_id;
			}

			if (close_flavour_id) {
				return cosm[id].get_flavour_id() == *close_flavour_id;
			}

			if (only_type_id) {
				return id.type_id != *only_type_id;
			}

			if (only_flavour_id) {
				return cosm[id].get_flavour_id() != *only_flavour_id;
			}

			return true;
		}
	);
}
