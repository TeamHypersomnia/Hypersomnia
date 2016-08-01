#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "game/transcendental/entity_id.h"
#include "game/detail/inventory_slot_id.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "augs/misc/stepped_timing.h"

namespace components {
	struct item_slot_transfers {
		struct mounting_operation {
			entity_id current_item;
			inventory_slot_id intented_mounting_slot;

			template <class Archive>
			void serialize(Archive& ar) {
				ar(
					CEREAL_NVP(current_item),
					CEREAL_NVP(intented_mounting_slot)
				);
			}
		} mounting;

		augs::constant_size_vector<entity_id, ONLY_PICK_THESE_ITEMS_COUNT> only_pick_these_items;
		bool pick_all_touched_items_if_list_to_pick_empty = true;

		augs::stepped_cooldown pickup_timeout = augs::stepped_cooldown(200);

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(mounting),

				CEREAL_NVP(only_pick_these_items),
				CEREAL_NVP(pick_all_touched_items_if_list_to_pick_empty),

				CEREAL_NVP(pickup_timeout)
			);
		}

		static mounting_operation find_suitable_montage_operation(const_entity_handle parent_container);

		void interrupt_mounting();
	};
}