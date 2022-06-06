#include "game/cosmos/cosmos.h"

#include "application/setups/debugger/property_debugger/fae/fae_tree_structs.h"
#include "game/cosmos/entity_handle.h"

bool fae_tree_filter::any() const {
	return deselect_type_id || deselect_flavour_id || select_only_type_id || select_only_flavour_id;
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
			if (deselect_type_id) {
				return id.type_id == *deselect_type_id;
			}

			if (deselect_flavour_id) {
				return cosm[id].get_flavour_id() == *deselect_flavour_id;
			}

			if (select_only_type_id) {
				return id.type_id != *select_only_type_id;
			}

			if (select_only_flavour_id) {
				return cosm[id].get_flavour_id() != *select_only_flavour_id;
			}

			return true;
		}
	);
}
