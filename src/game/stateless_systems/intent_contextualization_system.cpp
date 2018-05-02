#include "intent_contextualization_system.h"
#include "game/messages/intent_message.h"
#include "game/messages/motion_message.h"

#include "game/components/car_component.h"
#include "game/components/driver_component.h"
#include "game/components/gun_component.h"
#include "game/components/container_component.h"
#include "game/components/melee_component.h"
#include "game/components/hand_fuse_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"

#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/hand_fuse_logic.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

using namespace augs;

void intent_contextualization_system::contextualize_use_button_intents(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	auto& intents = step.get_queue<messages::intent_message>();
	
	for (auto& e : intents) {
		const auto subject = cosmos[e.subject];

		if (e.intent == game_intent_type::USE_BUTTON) {
			auto* const maybe_driver = subject.find<components::driver>();

			if (maybe_driver) {
				const auto car_id = maybe_driver->owned_vehicle;
				const auto car = cosmos[car_id];

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
		}
	}
}

void intent_contextualization_system::contextualize_crosshair_action_intents(const logic_step step) {
	auto& cosmos = step.get_cosmos();

	{
#if UNUSED
		auto& events = step.get_queue<messages::motion_message>();
		
		for (auto& it : events) {
			const auto subject = cosmos[it.subject];
		}
#endif
	}

	auto& events = step.get_queue<messages::intent_message>();

	for (auto& it : events) {
		entity_id callee;

		const auto subject = cosmos[it.subject];

		if (subject.find<invariants::container>()) {
			int hand_index = -1;

			if (it.intent == game_intent_type::CROSSHAIR_PRIMARY_ACTION) {
				hand_index = 0;
			}
			else if (it.intent == game_intent_type::CROSSHAIR_SECONDARY_ACTION) {
				hand_index = 1;
			}

			if (hand_index >= 0) {
				callee = subject.get_if_any_item_in_hand_no(static_cast<size_t>(hand_index));
			}
		}

		const auto callee_handle = cosmos[callee];

		if (callee_handle.alive()) {
			if (callee_handle.has<components::gun>()) {
				it.intent = game_intent_type::PRESS_GUN_TRIGGER;
				it.subject = callee;
				continue;
			}
			else if (callee_handle.has<components::melee>()) {
				if (it.intent == game_intent_type::CROSSHAIR_PRIMARY_ACTION) {
					it.intent = game_intent_type::MELEE_PRIMARY_MOVE;
				}
				else if (it.intent == game_intent_type::CROSSHAIR_SECONDARY_ACTION) {
					it.intent = game_intent_type::MELEE_SECONDARY_MOVE;
				}

				it.subject = callee;
				continue;
			}
			else if (callee_handle.has<components::hand_fuse>()) {
				release_or_throw_fused_object(
					step,
					cosmos[callee],
					subject,
					it.was_pressed()
				);
			}
		}
	}
}

void intent_contextualization_system::contextualize_movement_intents(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	auto& intents = step.get_queue<messages::intent_message>();

	for (auto& e : intents) {
		entity_id callee;
		bool callee_resolved = false;

		const auto subject = cosmos[e.subject];

		const auto* const maybe_driver = subject.find<components::driver>();
		const auto* const maybe_container = subject.find<invariants::container>();

		if (maybe_driver && cosmos[maybe_driver->owned_vehicle].alive()) {
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
			if (maybe_container) {
				if (e.intent == game_intent_type::SPACE_BUTTON) {
					const auto hand = subject.get_primary_hand();

					if (hand.alive() && hand.get_items_inside().size() > 0) {
						e.intent = game_intent_type::MELEE_TERTIARY_MOVE;
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
}
