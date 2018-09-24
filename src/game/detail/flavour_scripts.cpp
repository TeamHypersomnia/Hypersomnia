#include "game/detail/flavour_scripts.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

inventory_space_type calc_space_occupied_of_purchased(
	const cosmos& cosm, 
	const entity_flavour_id& id
) {
	return cosm.on_flavour(
		id,
		[&](const auto& typed_flavour) {
			if (const auto item = typed_flavour.template find<invariants::item>()) {
				if (const auto container = typed_flavour.template find<invariants::container>()) {
					if (const auto mag = mapped_or_nullptr(container->slots, slot_function::GUN_DETACHABLE_MAGAZINE)) {
						if (const auto& f = mag->only_allow_flavour; f.is_set()) {
							return item->space_occupied_per_charge + calc_space_occupied_of_purchased(cosm, f);
						}
					}
				}

				return item->space_occupied_per_charge;
			}

			return static_cast<inventory_space_type>(-1);
		}
	);
}
