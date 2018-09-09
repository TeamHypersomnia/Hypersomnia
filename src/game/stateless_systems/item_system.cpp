#include "augs/templates/container_templates.h"
#include "item_system.h"

#include "game/messages/intent_message.h"
#include "game/messages/collision_message.h"

#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/messages/queue_deletion.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"

#include "game/components/item_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sentience_component.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/entity_scripts.h"

#include "game/inferred_caches/physics_world_cache.h"
#include "game/cosmos/entity_handle.h"

#include "augs/ensure.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/enums/item_transfer_result_type.h"

#include "game/detail/physics/physics_scripts.h"

void item_system::pick_up_touching_items(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& clk = cosmos.get_clock();
	const auto& collisions = step.get_queue<messages::collision_message>();

	for (const auto& c : collisions) {
		if (c.type != messages::collision_message::event_type::PRE_SOLVE) {
			continue;
		}

		entity_id picker_id = c.subject;

		const auto picker = cosmos[picker_id];
		const auto item_entity = cosmos[c.collider];

		if (const auto item = item_entity.find<components::item>();
			item && item_entity.get_owning_transfer_capability().dead()
		) {
			picker.dispatch_on_having_all<components::item_slot_transfers>([&](const auto& typed_picker) {
				const auto& movement = typed_picker.template get<components::movement>();
				auto& transfers = typed_picker.template get<components::item_slot_transfers>();

				if (!movement.flags.picking) {
					return;
				}

				if (typed_picker.sentient_and_unconscious()) {
					return;
				}

				entity_id item_to_pick = item_entity;

				if (item_entity.get_current_slot().alive()) {
					item_to_pick = item_entity.get_current_slot().get_root_container();
				}

				const auto& pick_list = transfers.only_pick_these_items;
				const bool found_on_subscription_list = found_in(pick_list, item_to_pick);

				if (/* item_subscribed */
					(pick_list.empty() && transfers.pick_all_touched_items_if_list_to_pick_empty)
					|| found_on_subscription_list
				) {
					const auto pickup_slot = typed_picker.determine_pickup_target_slot_for(cosmos[item_to_pick]);

					if (pickup_slot.alive()) {
						const bool can_pick_already = transfers.pickup_timeout.try_to_fire_and_reset(clk);

						if (can_pick_already) {
							perform_transfer(item_slot_transfer_request::standard(item_to_pick, pickup_slot), step);
						}
					}
				}
			});
		}
	}
}

void item_system::handle_throw_item_intents(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& requests = step.get_queue<messages::intent_message>();

	for (auto r : requests) {
		if (r.was_pressed()) {
			auto requested_index = static_cast<std::size_t>(-1);

			if (r.intent == game_intent_type::THROW) {
				requested_index = 0;
			}
			else if (r.intent == game_intent_type::THROW_SECONDARY) {
				requested_index = 1;
			}

			if (requested_index != static_cast<std::size_t>(-1)) {
				const auto subject = cosmos[r.subject];

				if (subject.has<components::item_slot_transfers>()) {
					if (const auto item_inside = subject.calc_hand_action(requested_index).held_item; item_inside.is_set()) {
						perform_transfer(item_slot_transfer_request::drop(item_inside), step);
					}
				}
			}
		}
	}
}
