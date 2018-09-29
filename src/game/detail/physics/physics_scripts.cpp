#include "physics_scripts.h"
#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/item_component.h"
#include "game/components/driver_component.h"
#include "game/components/sentience_component.h"
#include "game/components/movement_component.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/inventory/inventory_slot_handle.h"

bool are_connected_by_friction(
	const const_entity_handle child, 
	const const_entity_handle parent
) {
	const auto& cosm = child.get_cosmos();

	bool matched_ancestor = false;

	const auto owner_body_of_parent = parent.get_owner_of_colliders();
	const auto owner_body_of_child = child.get_owner_of_colliders();

	if(owner_body_of_child.alive()) {
		entity_id childs_ancestor_entity = owner_body_of_child.get_owner_friction_ground();

		while (cosm[childs_ancestor_entity].alive()) {
			if (childs_ancestor_entity == owner_body_of_parent) {
				matched_ancestor = true;
				break;
			}

			childs_ancestor_entity = cosm[childs_ancestor_entity].get_owner_friction_ground();
		}
	}

	return matched_ancestor;
}