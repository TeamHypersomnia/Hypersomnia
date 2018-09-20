#pragma once
#include "game/detail/flavour_scripts.h"
#include "game/cosmos/entity_flavour_id.h"

template <class E>
int num_carryable_pieces(
	const E& in_subject,
	const slot_finding_opts& opts,
	const item_flavour_id& item
) {
	int total_fitting = 0;

	const auto& cosm = in_subject.get_cosmos();

	cosm.on_flavour(
		item,
		[&](const auto& typed_flavour) {
			const auto& item_def = typed_flavour.template get<invariants::item>();

			const auto piece_occupied_space = item_def.space_occupied_per_charge;

			auto check_slot = [&](const auto& slot) {
				if (slot.dead()) {
					return;
				}

				if (slot->is_category_compatible_with(item, item_def.categories_for_slot_compatibility)) {
					if (slot->always_allow_exactly_one_item) {
						if (slot.is_empty_slot()) {
							++total_fitting;
						}
					}
					else {
						total_fitting += slot.calc_real_space_available() / piece_occupied_space;
					}
				}
			};

			in_subject.for_each_candidate_slot(
				opts,
				check_slot
			);
		}
	);

	return total_fitting;
}
