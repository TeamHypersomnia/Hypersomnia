#include "game/detail/flavour_scripts.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

inventory_space_type calc_space_occupied_of_brand_new(
	const cosmos& cosm, 
	const entity_flavour_id& id
) {
	return cosm.on_flavour(
		id,
		[&](const auto& typed_flavour) {
			if (const auto item = typed_flavour.template find<invariants::item>()) {
#if 0
				if (const auto container = typed_flavour.template find<invariants::container>()) {
					if (const auto depo = mapped_or_nullptr(container->slots, slot_function::ITEM_DEPOSIT)) {
						if (depo->only_allow_flavour.is_set()) {
							/* It is a special item deposit which will be filled with something in particular */
							return depo->space_available + item->space_occupied_per_charge;
						}
					}
				}
#endif

				return item->space_occupied_per_charge;
			}

			return static_cast<inventory_space_type>(-1);
		}
	);
}
