#include "driver_system.h"
#include "../messages/car_ownership_change_message.h"
#include "../messages/trigger_hit_confirmation_message.h"

#include "../components/trigger_component.h"
#include "../components/car_component.h"
#include "../components/input_component.h"
#include "../components/children_component.h"
#include "../components/movement_component.h"

#include "../globals/input_profiles.h"

#include "entity_system/world.h"

void driver_system::assign_drivers_from_triggers() {
	auto& confirmations = parent_world.get_message_queue<messages::trigger_hit_confirmation_message>();

	for (auto& e : confirmations) {
		auto subject_car = e.trigger->get<components::trigger>().entity_to_be_notified;
		auto* maybe_car = subject_car->find<components::car>();

		if (maybe_car) {
			auto& car = *maybe_car;
			if (e.trigger == car.left_wheel_trigger) {
				if (car.current_driver.dead()) {
					messages::car_ownership_change_message msg;
					msg.car = subject_car;
					msg.driver = e.detector;
					parent_world.post_message(msg);
				}
			}
		}
	}
}

void driver_system::release_drivers_due_to_distance() {


}

void driver_system::release_drivers_due_to_requests() {
	auto& intents = parent_world.get_message_queue<messages::intent_message>();

	for (auto& e : intents) {
		if (e.intent == messages::intent_message::RELEASE_CAR && e.pressed_flag) {
			auto* maybe_driver = e.subject->find<components::driver>();

			if (maybe_driver) {
				auto car = maybe_driver->owned_vehicle;

				if (car.alive() && car->get<components::car>().current_driver == e.subject) {
					messages::car_ownership_change_message msg;
					msg.car = car;
					msg.driver = e.subject;
					msg.lost_ownership = true;
					parent_world.post_message(msg);
				}
			}
		}
	}
}

void driver_system::affect_drivers_due_to_car_ownership_changes() {
	auto& ownership_changes = parent_world.get_message_queue<messages::car_ownership_change_message>();

	for (auto& e : ownership_changes) {
		auto& car = e.car->get<components::car>();
		auto& driver = e.driver->get<components::driver>();

		if (!e.lost_ownership) {
			driver.owned_vehicle = e.car;
			car.current_driver = e.driver;

			auto* maybe_movement = e.driver->find<components::movement>();

			if (maybe_movement)
				maybe_movement->reset_movement_flags();

			auto* maybe_children = e.driver->find<components::children>();

			if (maybe_children) {
				auto& children = maybe_children->sub_entities_by_name;

				if (children[components::children::sub_entity_name::CHARACTER_CROSSHAIR].alive()) {
					//children[components::children::sub_entity_name::CHARACTER_CROSSHAIR]->get<components::input>() = input_profiles::crosshair_driving();
				}
			}
		}
		else {
			driver.owned_vehicle.unset();
			car.current_driver.unset();
			car.reset_movement_flags();

			auto* maybe_children = e.driver->find<components::children>();

			if (maybe_children) {
				auto& children = maybe_children->sub_entities_by_name;

				if (children[components::children::sub_entity_name::CHARACTER_CROSSHAIR].alive()) {
					//children[components::children::sub_entity_name::CHARACTER_CROSSHAIR]->get<components::input>() = input_profiles::crosshair();
				}
			}
		}
	}

	ownership_changes.clear();
}

void driver_system::delegate_movement_intents_from_drivers_to_steering_intents_of_owned_vehicles() {
	auto& intents = parent_world.get_message_queue<messages::intent_message>();

	for (auto& e : intents) {
		if (e.intent == messages::intent_message::MOVE_FORWARD
			|| e.intent == messages::intent_message::MOVE_BACKWARD
			|| e.intent == messages::intent_message::MOVE_LEFT
			|| e.intent == messages::intent_message::MOVE_RIGHT
			|| e.intent == messages::intent_message::HAND_BRAKE
			) {
			auto* maybe_driver = e.subject->find<components::driver>();

			if (maybe_driver) {
				if (maybe_driver->owned_vehicle.alive()) {
					e.subject = maybe_driver->owned_vehicle;
				}
			}
		}
	}
}