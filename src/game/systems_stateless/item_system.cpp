#include "item_system.h"

#include "game/messages/intent_message.h"
#include "game/messages/collision_message.h"

#include "game/detail/inventory/item_slot_transfer_request.h"
#include "game/messages/queue_destruction.h"

#include "game/transcendental/cosmos.h"

#include "game/components/item_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sentience_component.h"
#include "game/detail/gui/character_gui.h"

#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/entity_scripts.h"

#include "game/systems_inferred/physics_system.h"
#include "game/transcendental/entity_handle.h"

#include "augs/ensure.h"

#include "game/components/item_component.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/enums/item_transfer_result_type.h"

#include "game/detail/physics/physics_scripts.h"
#include "augs/templates/container_templates.h"

void item_system::start_picking_up_items(const logic_step step) {
	const auto& intents = step.transient.messages.get_queue<messages::intent_message>();
	auto& cosm = step.cosm;

	for (const auto& i : intents) {
		if (i.intent == intent_type::START_PICKING_UP_ITEMS) {
			const auto it = cosm[i.subject];

			const auto maybe_transfers = it.find<components::item_slot_transfers>();

			if (maybe_transfers != nullptr) {
				maybe_transfers->picking_up_touching_items_enabled = i.is_pressed;
			}
		}
	}
}

void item_system::pick_up_touching_items(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& collisions = step.transient.messages.get_queue<messages::collision_message>();

	for (const auto& c : collisions) {
		if (c.type != messages::collision_message::event_type::PRE_SOLVE) {
			continue;
		}

		entity_id picker_id = c.subject;

		const auto picker = cosmos[picker_id];
		const auto item = cosmos[c.collider];

		const auto* const maybe_item = item.find<components::item>();

		if (maybe_item != nullptr && item.get_owning_transfer_capability().dead()) {
			auto* maybe_transfers = picker.find<components::item_slot_transfers>();

			if (
				maybe_transfers != nullptr
				&& maybe_transfers->picking_up_touching_items_enabled	
			) {
				const auto actual_picker = cosmos[picker_id];

				if (actual_picker.has<components::sentience>()) {
					if (!actual_picker.get<components::sentience>().is_conscious()) {
						continue;
					}
				}

				entity_id item_to_pick = item;

				if (item.get_current_slot().alive()) {
					item_to_pick = item.get_current_slot().get_root_container();
				}

				const auto& pick_list = maybe_transfers->only_pick_these_items;
				const bool found_on_subscription_list = found_in(pick_list, item_to_pick);

				const bool item_subscribed = 
					(pick_list.empty() && maybe_transfers->pick_all_touched_items_if_list_to_pick_empty)
					|| found_on_subscription_list
				;

				if (item_subscribed) {
					const auto pickup_slot = actual_picker.determine_pickup_target_slot_for(cosmos[item_to_pick]);

					if (pickup_slot.alive()) {
						const bool can_pick_already = maybe_transfers->pickup_timeout.try_to_fire_and_reset(cosmos.get_timestamp(), delta);

						if (can_pick_already) {
							const item_slot_transfer_request request{ item_to_pick, pickup_slot };
							perform_transfer(request, step);
						}
					}
				}
			}
		}
	}
}

void item_system::handle_throw_item_intents(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& requests = step.transient.messages.get_queue<messages::intent_message>();

	for (auto r : requests) {
		if (r.is_pressed) {
			int hand_index = -1;

			if (r.intent == intent_type::THROW) {
				const auto subject = cosmos[r.subject];

				if (subject.get_if_any_item_in_hand_no(0).alive()) {
					r.intent = intent_type::THROW_PRIMARY_ITEM;
				}
				else if (
					subject.get_if_any_item_in_hand_no(0).dead()
					&& subject.get_if_any_item_in_hand_no(1).alive()
				) {
					r.intent = intent_type::THROW_SECONDARY_ITEM;
				}
			}

			if (r.intent == intent_type::THROW_PRIMARY_ITEM) {
				hand_index = 0;
			}
			else if (r.intent == intent_type::THROW_SECONDARY_ITEM) {
				hand_index = 1;
			}

			if (hand_index >= 0) {
				const auto subject = cosmos[r.subject];

				if (subject.has<components::item_slot_transfers>()) {
					const auto item_inside = subject.get_hand_no(static_cast<size_t>(hand_index)).get_item_if_any();

					if (item_inside.alive()) {
						perform_transfer({ item_inside, inventory_slot_id() }, step);
					}
				}
			}
		}
	}
}

void components::item_slot_transfers::interrupt_mounting() {
	mounting.current_item.unset();
	mounting.intented_mounting_slot.unset();
}

void item_system::process_mounting_and_unmounting(const logic_step step) {
	ensure(false);
	
	auto& cosmos = step.cosm;
	const auto delta = step.get_delta();
	
	cosmos.for_each(
		processing_subjects::WITH_ITEM_SLOT_TRANSFERS, 
		[&](const auto e) {
			auto& item_slot_transfers = e.get<components::item_slot_transfers>();

			const auto currently_mounted_item = cosmos[item_slot_transfers.mounting.current_item];

			if (currently_mounted_item.alive()) {
				auto& item = currently_mounted_item.get<components::item>();

				if (item.current_slot != item_slot_transfers.mounting.intented_mounting_slot) {
					item_slot_transfers.interrupt_mounting();
				}
				else {
					ensure(item.intended_mounting != item.current_mounting);

					if (item.montage_time_left_ms > 0) {
						item.montage_time_left_ms -= static_cast<float>(delta.in_milliseconds());
					}
					else {
						item.current_mounting = item.intended_mounting;

						if (item.current_mounting == components::item::UNMOUNTED) {
							perform_transfer({ currently_mounted_item, item.target_slot_after_unmount }, step);
						}
					}
				}
			}

			if (currently_mounted_item.dead()) {
				item_slot_transfers.mounting = components::item_slot_transfers::find_suitable_montage_operation(e);
			}
		}
	);
}
