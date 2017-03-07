#include "intent_contextualization_system.h"
#include "game/messages/intent_message.h"

#include "game/components/car_component.h"
#include "game/components/driver_component.h"
#include "game/components/gun_component.h"
#include "game/components/container_component.h"
#include "game/components/trigger_query_detector_component.h"
#include "game/components/melee_component.h"
#include "game/components/trigger_collision_detector_component.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"

#include "game/detail/inventory/inventory_slot_id.h"
#include "game/detail/inventory/inventory_slot_handle.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

using namespace augs;

void intent_contextualization_system::contextualize_use_button_intents(const logic_step step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto& intents = step.transient.messages.get_queue<messages::intent_message>();
	
	for (auto& e : intents) {
		const auto subject = cosmos[e.subject];

		const auto* const query_detector = subject.find<components::trigger_query_detector>();
		const auto* const collision_detector = subject.find<components::trigger_collision_detector>();
		
		if (e.intent == intent_type::USE_BUTTON) {
			const auto* const maybe_driver = subject.find<components::driver>();

			if (maybe_driver) {
				const auto car_id = maybe_driver->owned_vehicle;
				const auto car = cosmos[car_id];

				if (car.alive() && car.get<components::car>().current_driver == e.subject) {
					e.intent = intent_type::RELEASE_CAR;
					continue;
				}
			}

			if (query_detector) {
				e.intent = intent_type::QUERY_TOUCHING_TRIGGERS;
				continue;
			}
		}
		else if (e.intent == intent_type::START_PICKING_UP_ITEMS) {
			if (collision_detector) {
				e.intent = intent_type::DETECT_TRIGGER_COLLISIONS;
				continue;
			}
		}
	}
}

void intent_contextualization_system::contextualize_crosshair_action_intents(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	auto& events = step.transient.messages.get_queue<messages::intent_message>();

	for (auto& it : events) {
		entity_id callee;

		const auto subject = cosmos[it.subject];

		const auto maybe_crosshair = subject[child_entity_name::CHARACTER_CROSSHAIR];

		if (it.intent == intent_type::MOVE_CROSSHAIR && maybe_crosshair.alive()) {
			it.subject = maybe_crosshair;
			continue;
		}

		if (subject.has<components::container>()) {
			if (it.intent == intent_type::CROSSHAIR_PRIMARY_ACTION) {
				const auto hand = subject[slot_function::PRIMARY_HAND];

				if (hand.alive() && hand->items_inside.size() > 0)
					callee = hand.get_items_inside()[0];
			}

			if (it.intent == intent_type::CROSSHAIR_SECONDARY_ACTION) {
				const auto hand = subject[slot_function::SECONDARY_HAND];

				if (hand.alive() && hand->items_inside.size() > 0)
					callee = hand.get_items_inside()[0];
				else {
					const auto prim_hand = subject[slot_function::PRIMARY_HAND];

					if (prim_hand.alive() && prim_hand->items_inside.size() > 0)
						callee = prim_hand.get_items_inside()[0];
				}
			}
		}

		const auto callee_handle = cosmos[callee];

		if (callee_handle.alive()) {
			if (callee_handle.find<components::gun>()) {
				it.intent = intent_type::PRESS_GUN_TRIGGER;
				it.subject = callee;
				continue;
			}
			if (callee_handle.find<components::melee>()) {
				if (it.intent == intent_type::CROSSHAIR_PRIMARY_ACTION) {
					it.intent = intent_type::MELEE_PRIMARY_MOVE;
				}
				else if (it.intent == intent_type::CROSSHAIR_SECONDARY_ACTION) {
					it.intent = intent_type::MELEE_SECONDARY_MOVE;
				}

				it.subject = callee;
				continue;
			}
		}
	}
}

void intent_contextualization_system::contextualize_movement_intents(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	auto& intents = step.transient.messages.get_queue<messages::intent_message>();

	for (auto& e : intents) {
		entity_id callee;
		bool callee_resolved = false;

		const auto subject = cosmos[e.subject];

		const auto* const maybe_driver = subject.find<components::driver>();
		const auto* const maybe_container = subject.find<components::container>();

		if (maybe_driver && cosmos[maybe_driver->owned_vehicle].alive()) {
			if (e.intent == intent_type::MOVE_FORWARD
				|| e.intent == intent_type::MOVE_BACKWARD
				|| e.intent == intent_type::MOVE_LEFT
				|| e.intent == intent_type::MOVE_RIGHT
				|| e.intent == intent_type::WALK
				|| e.intent == intent_type::SPRINT
				) {
				callee = maybe_driver->owned_vehicle;
				callee_resolved = true;
			}
			else if (e.intent == intent_type::SPACE_BUTTON) {
				callee = maybe_driver->owned_vehicle;
				callee_resolved = true;
				e.intent = intent_type::HAND_BRAKE;
			}
		}
		
		if (!callee_resolved) {
			if (maybe_container) {
				if (e.intent == intent_type::SPACE_BUTTON) {
					const auto hand = subject[slot_function::PRIMARY_HAND];

					if (hand.alive() && hand->items_inside.size() > 0) {
						e.intent = intent_type::MELEE_TERTIARY_MOVE;
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
