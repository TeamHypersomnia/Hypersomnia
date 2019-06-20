#include "intent_contextualization_system.h"
#include "game/messages/intent_message.h"
#include "game/messages/motion_message.h"

#include "game/components/car_component.h"
#include "game/components/driver_component.h"
#include "game/components/gun_component.h"
#include "game/components/container_component.h"
#include "game/components/melee_component.h"
#include "game/components/hand_fuse_component.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/for_each_entity.h"

#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/hand_fuse_logic.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/start_defusing_nearby_bomb.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/detail/weapon_like.h"

using namespace augs;

void intent_contextualization_system::handle_use_button_presses(const logic_step step) {
	auto& cosm = step.get_cosmos();
	auto& intents = step.get_queue<messages::intent_message>();
	
	for (auto& e : intents) {
		const auto subject = cosm[e.subject];

		if (e.intent == game_intent_type::USE) {
			if (const auto sentience = subject.find<components::sentience>()) {
				auto& u = sentience->use_button;

				if (e.was_pressed()) {
					if (u == use_button_state::IDLE) {
						u = use_button_state::QUERYING;
					}
				}
				else {
					u = use_button_state::IDLE;
				}
			}
		}
	}
}

void intent_contextualization_system::advance_use_button(const logic_step step) {
	auto& cosm = step.get_cosmos();

	cosm.for_each_having<components::sentience>(
		[&](const auto subject) {
			auto& sentience = subject.template get<components::sentience>();

			if (sentience.use_button == use_button_state::QUERYING) {
				if (const auto transform = subject.find_logic_transform()) {
					auto& u = sentience.last_use_result;

					const auto result = query_defusing_nearby_bomb(subject);
					u = result.result;

					if (result.success()) {
						result.perform(cosm);
						sentience.use_button = use_button_state::DEFUSING;
						return;
					}
				}
			}
			else {
				sentience.last_use_result = use_button_query_result::NONE_FOUND;
			}

			{
#if TODO_CARS
				auto* const maybe_driver = subject.find<components::driver>();

				if (maybe_driver) {
					const auto car_id = maybe_driver->owned_vehicle;
					const auto car = cosm[car_id];

					const bool is_now_driving = 
						car.alive() 
						&& car.get<components::car>().current_driver == e.subject
					;

					if (is_now_driving) {
						e.intent = game_intent_type::RELEASE_CAR;
					}
					else {
						e.intent = game_intent_type::TAKE_HOLD_OF_WHEEL;
					}
				}
#endif
			}
		}
	);
}

void intent_contextualization_system::contextualize_crosshair_action_intents(const logic_step step) {
	auto& cosm = step.get_cosmos();

	{
#if UNUSED
		auto& events = step.get_queue<messages::motion_message>();
		
		for (auto& it : events) {
			const auto subject = cosm[it.subject];
		}
#endif
	}

	auto& events = step.get_queue<messages::intent_message>();

	for (auto& it : events) {
		entity_id callee;

		const auto subject = cosm[it.subject];

		if (subject.is_frozen()) {
			continue;
		}

		auto action_type = weapon_action_type::COUNT;

		subject.dispatch_on_having_all<components::sentience>([&](const auto& typed_subject) {
			auto requested_index = static_cast<std::size_t>(-1);

			if (it.intent == game_intent_type::CROSSHAIR_PRIMARY_ACTION) {
				requested_index = 0;
			}
			else if (it.intent == game_intent_type::CROSSHAIR_SECONDARY_ACTION) {
				requested_index = 1;
			}

			if (requested_index != static_cast<std::size_t>(-1)) {
				const auto& idx = requested_index;

				const auto action = subject.calc_hand_action(idx);
				callee = action.held_item;
				action_type = action.type;

				auto& sentience = typed_subject.template get<components::sentience>();
				sentience.hand_flags[idx] = it.was_pressed();

				if (it.was_pressed()) {
					sentience.when_hand_pressed[idx] = cosm.get_timestamp();
				}
			}
		});

		const auto callee_handle = cosm[callee];

		if (callee_handle.alive()) {
			callee_handle.dispatch_on_having_all<components::gun>(
				[&](const auto& typed_gun) {
					typed_gun.template get<components::gun>().just_pressed[action_type] = true;
				}
			);

			callee_handle.dispatch_on_having_all<invariants::item>(
				[&](const auto& typed_item) {
					if (::is_armor_like(typed_item)) {
						if (const auto armor_slot = subject[slot_function::TORSO_ARMOR]) {
							if (armor_slot.is_empty_slot()) {
								if (it.was_pressed() && typed_item.find_mounting_progress() == nullptr) {
									perform_transfer(item_slot_transfer_request::standard(typed_item, armor_slot), step);
								}
							}
						}
					}
				}
			);

			callee_handle.dispatch_on_having_all<components::hand_fuse>(
				[&](const auto typed_fused) {
					const auto fuse_logic = fuse_logic_provider(typed_fused, step);

					if (fuse_logic.fuse_def.has_delayed_arming()) {
						fuse_logic.fuse.arming_requested = it.was_pressed();
					}
					else {
						if (it.was_pressed()) {
							fuse_logic.arm_explosive();
							fuse_logic.fuse.armed_as_secondary_action = action_type == weapon_action_type::SECONDARY;
						}
						else {
							fuse_logic.release_explosive_if_armed();
						}
					}
				}
			);
		}
	}
}

void intent_contextualization_system::contextualize_movement_intents(const logic_step step) {
#if LEGACY
	auto& cosm = step.get_cosmos();
	auto& intents = step.get_queue<messages::intent_message>();

	for (auto& e : intents) {
		entity_id callee;
		bool callee_resolved = false;

		const auto subject = cosm[e.subject];


		const auto* const maybe_driver = subject.find<components::driver>();

		if (maybe_driver && cosm[maybe_driver->owned_vehicle].alive()) {
			if (e.intent == game_intent_type::MOVE_FORWARD
				|| e.intent == game_intent_type::MOVE_BACKWARD
				|| e.intent == game_intent_type::MOVE_LEFT
				|| e.intent == game_intent_type::MOVE_RIGHT
				|| e.intent == game_intent_type::WALK
				|| e.intent == game_intent_type::SPRINT
			) {
				callee = maybe_driver->owned_vehicle;
				callee_resolved = true;
			}
			else if (e.intent == game_intent_type::SPACE_BUTTON) {
				callee = maybe_driver->owned_vehicle;
				callee_resolved = true;
				e.intent = game_intent_type::HAND_BRAKE;
			}
		}

		if (!callee_resolved) {
			if (const auto* const maybe_container = subject.find<invariants::container>();) {
				if (e.intent == game_intent_type::SPACE_BUTTON) {
					const auto hand = subject.get_primary_hand();

					if (hand.alive() && hand.get_items_inside().size() > 0) {
						callee = hand.get_items_inside()[0];
						callee_resolved = true;
					}
				}
			}
		}

		if (callee_resolved) {
			e.subject = callee;
		}
	}
#else
	(void)step;
#endif
}
