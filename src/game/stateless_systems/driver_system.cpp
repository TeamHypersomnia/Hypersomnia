#include "driver_system.h"
#include "game/messages/collision_message.h"
#include "game/messages/intent_message.h"

#include "game/components/car_component.h"

#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/sentience_component.h"

#include "game/transcendental/cosmos.h"
#include "game/inferred_caches/physics_world_cache.h"

#include "game/components/driver_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"

#include "game/detail/physics/physics_scripts.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

void driver_system::assign_drivers_who_touch_wheels(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& contacts = step.transient.messages.get_queue<messages::collision_message>();

	for (const auto& e : contacts) {
		if (!(e.type == messages::collision_message::event_type::PRE_SOLVE)) {
			continue;
		}

		const auto driver = cosmos[e.subject];
		const auto maybe_driver = driver.find<components::driver>();

		if (maybe_driver != nullptr && maybe_driver->take_hold_of_wheel_when_touched) {
			const auto car = cosmos[e.collider].get_owner_body();
			const auto maybe_car = car.find<components::car>();

			if(maybe_car != nullptr) {
				if (e.collider == maybe_car->left_wheel_trigger) {
					assign_car_ownership(driver, car);
				}
			}
		}
	}
}

void driver_system::release_drivers_due_to_ending_contact_with_wheel(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& contacts = step.transient.messages.get_queue<messages::collision_message>();
	const auto& physics = cosmos.inferential.physics;

	for (const auto& c : contacts) {
		if (c.type == messages::collision_message::event_type::END_CONTACT) {
			const auto& driver = cosmos[c.subject];
			const auto& car = cosmos[c.collider].get_owner_body();

			const auto* const maybe_driver = driver.find<components::driver>();

			if (maybe_driver) {
				if (maybe_driver->owned_vehicle == car) {
					release_car_ownership(driver);
					driver.get<components::movement>().make_inert_for_ms = 500.f;
				}
			}
		}
	}
}
void driver_system::release_drivers_due_to_requests(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& intents = step.transient.messages.get_queue<messages::intent_message>();

	for (const auto& e : intents) {
		if (e.intent == game_intent_type::RELEASE_CAR && e.was_pressed()) {
			release_car_ownership(cosmos[e.subject]);
		}
		else if (e.intent == game_intent_type::TAKE_HOLD_OF_WHEEL) {
			const auto subject = cosmos[e.subject];

			const auto* const sentience = subject.find<components::sentience>();

			if (sentience && !sentience->is_conscious()) {
				continue;
			}

			auto* const maybe_driver = subject.find<components::driver>();

			if (maybe_driver != nullptr) {
				maybe_driver->take_hold_of_wheel_when_touched = e.was_pressed();
			}
		}
	}
}

bool driver_system::release_car_ownership(const entity_handle driver) {
	return change_car_ownership(driver, driver.get_cosmos()[entity_id()]);
}

bool driver_system::assign_car_ownership(const entity_handle driver, const entity_handle car) {
	return change_car_ownership(driver, car);
}

bool driver_system::change_car_ownership(
	const entity_handle driver_entity, 
	const entity_handle car_entity
) {
	auto& driver = driver_entity.get<components::driver>();
	auto& cosmos = driver_entity.get_cosmos();
	const auto& physics = cosmos.inferential.physics;

	auto* const maybe_rotation_copying = driver_entity.find<components::rotation_copying>();
	const auto maybe_rigid_body = driver_entity.find<components::rigid_body>();
	const bool has_physics = maybe_rigid_body != nullptr;
	auto* const maybe_movement = driver_entity.find<components::movement>();
	auto force_joint = driver_entity.get<components::motor_joint>().get_raw_component();

	if (const bool new_ownership = car_entity.alive()) {
		auto& car = car_entity.get<components::car>();

		if (cosmos[car.current_driver].alive()) {
			return false;
		}

		// reset the input flag so it is necessary to press the key again
		driver.take_hold_of_wheel_when_touched = false;

		driver.owned_vehicle = car_entity;
		car.current_driver = driver_entity;

		car.last_turned_on = cosmos.get_timestamp();

		force_joint.target_bodies[0] = driver_entity;
		force_joint.target_bodies[1] = car.left_wheel_trigger;
		force_joint.activated = true;
		driver_entity.get<components::motor_joint>() = force_joint;

		if (maybe_movement) {
			maybe_movement->reset_movement_flags();
			maybe_movement->enable_braking_damping = false;
			maybe_movement->enable_animation = false;
		}

		if (maybe_rotation_copying && has_physics) {
			maybe_rotation_copying->stash();
			maybe_rotation_copying->target = car.left_wheel_trigger;
			maybe_rotation_copying->look_mode = components::rotation_copying::look_type::ROTATION;
		}

		if (has_physics) {
			//maybe_rigid_body.set_transform(car.left_wheel_trigger);
			//maybe_rigid_body.set_velocity(vec2(0, 0));
			resolve_density_of_associated_fixtures(driver_entity);
		}
	}
	else if (
		const auto owned_vehicle = cosmos[driver.owned_vehicle];
		owned_vehicle.alive()
	) {
		auto& car = owned_vehicle.get<components::car>();

		car.last_turned_off = cosmos.get_timestamp();

		driver.owned_vehicle.unset();
		car.current_driver.unset();

		force_joint.activated = false;
		driver_entity.get<components::motor_joint>() = force_joint;

		if (maybe_movement) {
			maybe_movement->reset_movement_flags();
			maybe_movement->enable_braking_damping = true;
			maybe_movement->enable_animation = true;

			maybe_movement->moving_left = car.turning_left;
			maybe_movement->moving_right = car.turning_right;
			maybe_movement->moving_forward = car.accelerating;
			maybe_movement->moving_backward = car.decelerating;
		}
		
		car.reset_movement_flags();

		if (maybe_rotation_copying && has_physics) {
			maybe_rotation_copying->unstash();
		}

		if (has_physics) {
			resolve_density_of_associated_fixtures(driver_entity);
		}
	}

	return true;
}
