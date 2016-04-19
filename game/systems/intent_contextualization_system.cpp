#include "intent_contextualization_system.h"
#include "../messages/intent_message.h"

#include "../components/car_component.h"
#include "../components/driver_component.h"
#include "../components/gun_component.h"
#include "../components/container_component.h"
#include "../components/trigger_query_detector_component.h"
#include "../components/trigger_collision_detector_component.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"

#include "../detail/inventory_slot_id.h"

void intent_contextualization_system::contextualize_use_button_intents() {
	auto& intents = parent_world.get_message_queue<messages::intent_message>();
	
	for (auto& e : intents) {
		auto* query_detector = e.subject->find<components::trigger_query_detector>();
		auto* collision_detector = e.subject->find<components::trigger_collision_detector>();
		
		if (e.intent == intent_type::USE_BUTTON) {
			auto* maybe_driver = e.subject->find<components::driver>();

			if (maybe_driver) {
				auto car = maybe_driver->owned_vehicle;

				if (car.alive() && car->get<components::car>().current_driver == e.subject) {
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

void intent_contextualization_system::contextualize_crosshair_action_intents() {
	auto& events = parent_world.get_message_queue<messages::intent_message>();

	for (auto& it : events) {
		augs::entity_id callee;

		auto* maybe_container = it.subject->find<components::container>();

		if (maybe_container) {
			if (it.intent == intent_type::CROSSHAIR_PRIMARY_ACTION) {
				auto hand = it.subject[slot_function::PRIMARY_HAND];

				if (hand.alive() && hand->items_inside.size() > 0)
					callee = hand->items_inside[0];
			}

			if (it.intent == intent_type::CROSSHAIR_SECONDARY_ACTION) {
				auto hand = it.subject[slot_function::SECONDARY_HAND];

				if (hand.alive() && hand->items_inside.size() > 0)
					callee = hand->items_inside[0];
				else {
					hand = it.subject[slot_function::PRIMARY_HAND];

					if (hand.alive() && hand->items_inside.size() > 0)
						callee = hand->items_inside[0];
				}
			}
		}

		if (callee.alive()) {
			if (callee->find<components::gun>()) {
				it.intent = intent_type::PRESS_GUN_TRIGGER;
				it.subject = callee;
				continue;
			}
			if (callee->find<components::melee>()) {
				if (it.intent == intent_type::CROSSHAIR_PRIMARY_ACTION)
					it.intent = intent_type::MELEE_PRIMARY_MOVE;
				else if (it.intent == intent_type::CROSSHAIR_SECONDARY_ACTION)
					it.intent = intent_type::MELEE_SECONDARY_MOVE;

				it.subject = callee;
				continue;
			}
		}
	}
}

void intent_contextualization_system::contextualize_movement_intents() {
	auto& intents = parent_world.get_message_queue<messages::intent_message>();

	for (auto& e : intents) {
		augs::entity_id callee;

		auto* maybe_driver = e.subject->find<components::driver>();
		auto* maybe_container = e.subject->find<components::container>();

		if (maybe_driver && maybe_driver->owned_vehicle.alive()) {
			if (e.intent == intent_type::MOVE_FORWARD
				|| e.intent == intent_type::MOVE_BACKWARD
				|| e.intent == intent_type::MOVE_LEFT
				|| e.intent == intent_type::MOVE_RIGHT) {
				callee = maybe_driver->owned_vehicle;
			}
			else if (e.intent == intent_type::SPACE_BUTTON) {
				callee = maybe_driver->owned_vehicle;
				e.intent = intent_type::HAND_BRAKE;
			}
		}
		
		if (callee.dead()) {
			if (maybe_container) {
				if (e.intent == intent_type::SPACE_BUTTON) {
					auto hand = e.subject[slot_function::PRIMARY_HAND];

					if (hand.alive() && hand->items_inside.size() > 0) {
						e.intent = intent_type::MELEE_TERTIARY_MOVE;
						callee = hand->items_inside[0];
					}
				}
			}
		}

		if(callee.alive())
			e.subject = callee;
	}
}
