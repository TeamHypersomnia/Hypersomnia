#include "driver_system.h"
#include "../messages/car_ownership_change_message.h"
#include "../messages/trigger_hit_confirmation_message.h"
#include "../messages/collision_message.h"

#include "../components/trigger_component.h"
#include "../components/car_component.h"
#include "../components/input_receiver_component.h"
#include "../components/children_component.h"
#include "../components/movement_component.h"
#include "../components/rotation_copying_component.h"
#include "../components/physics_component.h"

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
					affect_drivers_due_to_car_ownership_changes(msg);
				}
			}
		}
	}
}

void driver_system::release_drivers_due_to_distance() {
	auto& contacts = parent_world.get_message_queue<messages::collision_message>();

	for (auto& c : contacts) {
		if (c.sensor_end_contact) {
			auto driver = components::physics::get_owner_body_entity(c.subject);
			auto car = components::physics::get_owner_body_entity(c.collider);

			auto* maybe_driver = driver->find<components::driver>();

			if (maybe_driver) {
				if (maybe_driver->owned_vehicle == car) {
					messages::car_ownership_change_message msg;
					msg.car = car;
					msg.driver = driver;
					msg.lost_ownership = true;
					affect_drivers_due_to_car_ownership_changes(msg);

					msg.driver->get<components::movement>().make_inert_for_ms = 500.f;
				}
			}
		}
	}
}

void driver_system::release_drivers_due_to_requests() {
	auto& intents = parent_world.get_message_queue<messages::intent_message>();

	for (auto& e : intents) {
		if (e.intent == intent_type::RELEASE_CAR && e.pressed_flag) {
			auto* maybe_driver = e.subject->find<components::driver>();

			if (maybe_driver) {
				auto car = maybe_driver->owned_vehicle;

				if (car.alive() && car->get<components::car>().current_driver == e.subject) {
					messages::car_ownership_change_message msg;
					msg.car = car;
					msg.driver = e.subject;
					msg.lost_ownership = true;
					affect_drivers_due_to_car_ownership_changes(msg);
					
					e.clear();
				}
			}
		}
	}
}

void driver_system::affect_drivers_due_to_car_ownership_changes(messages::car_ownership_change_message& e) {
	auto& car = e.car->get<components::car>();
	auto& driver = e.driver->get<components::driver>();

	auto* maybe_children = e.driver->find<components::children>();
	auto* maybe_rotation_copying = e.driver->find<components::rotation_copying>();
	auto* maybe_physics = e.driver->find<components::physics>();
	auto* maybe_movement = e.driver->find<components::movement>();

	if (!e.lost_ownership) {
		driver.owned_vehicle = e.car;
		car.current_driver = e.driver;

		if (maybe_movement) {
			maybe_movement->reset_movement_flags();
			maybe_movement->enable_braking_damping = false;
			maybe_movement->enable_animation = false;
		}

		if (maybe_children) {
			auto& children = maybe_children->sub_entities_by_name;

			if (children[components::children::sub_entity_name::CHARACTER_CROSSHAIR].alive()) {
				//children[components::children::sub_entity_name::CHARACTER_CROSSHAIR]->get<components::input>() = input_profiles::crosshair_driving();
			}
		}

		if (maybe_rotation_copying && maybe_physics) {
			maybe_rotation_copying->update_value = false;
			//maybe_physics->enable_angle_motor = false;
		}

		if (maybe_physics) {
			maybe_physics->set_transform(car.left_wheel_trigger);
			maybe_physics->set_velocity(vec2(0, 0));
			maybe_physics->set_density(driver.density_while_driving);

			//maybe_physics->set_linear_damping(driver.linear_damping_while_driving);
		}
	}
	else {
		driver.owned_vehicle.unset();
		car.current_driver.unset();

		if (maybe_movement) {
			maybe_movement->reset_movement_flags();
			maybe_movement->enable_braking_damping = true;
			maybe_movement->enable_animation = true;

			maybe_movement->moving_left = car.turning_left;
			maybe_movement->moving_right = car.turning_right;
			maybe_movement->moving_forward = car.accelerating;
			maybe_movement->moving_backward = car.deccelerating;
		}
		
		car.reset_movement_flags();

		if (maybe_rotation_copying && maybe_physics) {
			maybe_rotation_copying->update_value = true;
			//maybe_physics->enable_angle_motor = true;
		}

		if (maybe_physics) {
			maybe_physics->set_density(driver.standard_density);
			// maybe_physics->set_linear_damping(driver.standard_linear_damping);
		}

		if (maybe_children) {
			auto& children = maybe_children->sub_entities_by_name;

			if (children[components::children::sub_entity_name::CHARACTER_CROSSHAIR].alive()) {
				//children[components::children::sub_entity_name::CHARACTER_CROSSHAIR]->get<components::input>() = input_profiles::crosshair();
			}
		}
	}
}

void driver_system::delegate_movement_intents_from_drivers_to_steering_intents_of_owned_vehicles() {
	auto& intents = parent_world.get_message_queue<messages::intent_message>();

	for (auto& e : intents) {
		if (e.intent == intent_type::MOVE_FORWARD
			|| e.intent == intent_type::MOVE_BACKWARD
			|| e.intent == intent_type::MOVE_LEFT
			|| e.intent == intent_type::MOVE_RIGHT
			|| e.intent == intent_type::HAND_BRAKE
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

void driver_system::apply_forces_towards_wheels() {
	for (auto& it : targets) {
		auto& physics = it->get<components::physics>();
		auto& driver = it->get<components::driver>();

		if (driver.owned_vehicle.alive()) {
			auto& car = driver.owned_vehicle->get<components::car>();
			auto direction = car.left_wheel_trigger->get<components::transform>().pos - physics.get_position();
			auto distance = direction.length();
			direction.normalize_hint(distance);

			float force_length = driver.force_towards_owned_wheel;

			if (distance < driver.distance_when_force_easing_starts) {
				auto mult = distance / driver.distance_when_force_easing_starts;
				force_length *= pow(mult, driver.power_of_force_easing_multiplier);
			}

			auto force = vec2(direction).set_length(force_length);

			physics.apply_force(force * physics.get_mass());
			physics.target_angle = car.left_wheel_trigger->get<components::transform>().rotation - 90;
		}
	}
}
