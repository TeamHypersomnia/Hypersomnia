#pragma once
#include "game/organization/all_entity_types_declaration.h"
#include "application/setups/editor/commands/change_flavour_property_command.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

class cosmos;

struct invariant_field_eq_predicate {
	const cosmos& cosm;
	const unsigned invariant_id;
	const entity_type_id type_id;
	const affected_flavours_type& ids;
	
	template <class M>
	bool operator()(
		const M& first,
	   	const field_address field_id
	) const {
		if (ids.size() == 1) {
			return true;
		}

		flavour_property_id property_id;
		property_id.invariant_id = invariant_id;
		property_id.field = field_id;

		return compare_all_fields_to(first, property_id, type_id, cosm, ids);
	}
};
