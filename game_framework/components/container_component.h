#pragma once
#include <unordered_map>
#include "entity_system/entity_id.h"

#include "../globals/inventory.h"

#include "transform_component.h"

namespace components {
	struct container {
		static unsigned calculate_space_occupied_with_children(augs::entity_id item);

		struct slot;
		struct slot_id {
			slot_function type = slot_function::ITEM_DEPOSIT;
			augs::entity_id container_entity;

			bool alive();
			bool dead();

			void unset();

			void add_item(augs::entity_id);
			void remove_item(augs::entity_id);

			unsigned calculate_free_space_with_parent_containers();
			bool can_contain(augs::entity_id);

			bool is_hand_slot();
			bool should_item_inside_keep_physical_body();

			slot& operator*();
			slot* operator->();
		};

		struct slot {
			bool holsterable = true;

			bool for_categorized_items_only = false;
			unsigned long long category_allowed = 0;

			unsigned transfer_speed_multiplier = 1;
			unsigned space_available = 7;

			bool disregard_space_and_allow_one_entity = false;

			components::transform attachment_local_offset;

			std::vector<augs::entity_id> items_inside;

			unsigned calculate_free_space_with_children();
		};

		std::unordered_map<slot_function, slot> slots;
	};
}