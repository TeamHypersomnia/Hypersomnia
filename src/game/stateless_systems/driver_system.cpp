#include "driver_system.h"
#include "game/messages/collision_message.h"
#include "game/messages/intent_message.h"

#include "game/components/car_component.h"

#include "game/components/movement_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/force_joint_component.h"
#include "game/components/sentience_component.h"

#include "game/cosmos/cosmos.h"
#include "game/inferred_caches/physics_world_cache.h"

#include "game/components/driver_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"

#include "game/detail/physics/physics_scripts.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"

void driver_system::assign_drivers_who_touch_wheels(const logic_step step) {
	(void)step;
#if TODO_CARS
	auto& cosmos = step.get_cosmos();
	const auto& contacts = step.get_queue<messages::collision_message>();

	for (const auto& e : contacts) {
		if (!(e.type == messages::collision_message::event_type::PRE_SOLVE)) {
			continue;
		}

		const auto driver = cosmos[e.subject];

		if (const auto maybe_driver = driver.find<components::driver>();
			maybe_driver != nullptr && maybe_driver->take_hold_of_wheel_when_touched
		) {
			if (driver.sentient_and_unconscious()) {
				continue;
			}

			const auto car = cosmos[e.collider].get_owner_of_colliders();
			const auto maybe_car = car.find<components::car>();

			if(maybe_car != nullptr) {
				if (e.collider == maybe_car->left_wheel_trigger) {
					assign_car_ownership(driver, car);
				}
			}
		}
	}
#endif
}

void driver_system::release_drivers_due_to_ending_contact_with_wheel(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& contacts = step.get_queue<messages::collision_message>();

	for (const auto& c : contacts) {
		if (c.type == messages::collision_message::event_type::END_CONTACT) {
			const auto& driver_entity = cosmos[c.subject];
			const auto car_entity = cosmos[c.collider].get_owner_of_colliders();

			if (car_entity.alive()) {
				if (const auto* const driver = driver_entity.find<components::driver>()) {
					if (driver->owned_vehicle == car_entity) {
						release_car_ownership(driver_entity);
						driver_entity.get<components::movement>().make_inert_for_ms = 500.f;
					}
				}
			}
		}
	}
}

void driver_system::release_drivers_due_to_requests(const logic_step step) {
#if TODO_CARS
	auto& cosmos = step.get_cosmos();
	const auto& intents = step.get_queue<messages::intent_message>();

	for (const auto& e : intents) {
		if (e.intent == game_intent_type::RELEASE_CAR && e.was_pressed()) {
			release_car_ownership(cosmos[e.subject]);
		}
		else if (e.intent == game_intent_type::TAKE_HOLD_OF_WHEEL) {
			const auto subject = cosmos[e.subject];

			if (subject.sentient_and_unconscious()) {
				continue;
			}

			auto* const maybe_driver = subject.find<components::driver>();

			if (maybe_driver != nullptr) {
				maybe_driver->take_hold_of_wheel_when_touched = e.was_pressed();
			}
		}
	}
#endif
	(void)step;
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

	const auto maybe_rigid_body = driver_entity.find<components::rigid_body>();
	const bool has_physics = maybe_rigid_body != nullptr;
	auto* const movement = driver_entity.find<components::movement>();

	if (/* new_ownership */ car_entity.alive()) {
		auto& car = car_entity.get<components::car>();

		if (cosmos[car.current_driver].alive()) {
			return false;
		}

		// reset the input flag so it is necessary to press the key again
#if TODO
		driver.take_hold_of_wheel_when_touched = false;
#endif

		driver.owned_vehicle = car_entity;
		car.current_driver = driver_entity;

		car.last_turned_on = cosmos.get_timestamp();

#if TODO
		force_joint.target_bodies[0] = driver_entity;
		force_joint.target_bodies[1] = car.left_wheel_trigger;
		force_joint.activated = true;
#endif

		if (movement) {
			movement->reset_movement_flags();
#if TODO
			movement->enable_braking_damping = false;
#endif
		}

		if (has_physics) {
			//maybe_rigid_body.set_transform(car.left_wheel_trigger);
			//maybe_rigid_body.set_velocity(vec2(0, 0));
#if TODO
			resolve_density_of_associated_fixtures(driver_entity);
#endif
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

#if TODO
		force_joint.activated = false;
#endif

		if (movement) {
			movement->reset_movement_flags();
#if TODO
			movement->enable_braking_damping = true;
#endif

			movement->flags.left = car.turning_left;
			movement->flags.right = car.turning_right;
			movement->flags.forward = car.accelerating;
			movement->flags.backward = car.decelerating;
		}
		
		car.reset_movement_flags();

		if (has_physics) {
#if TODO
			resolve_density_of_associated_fixtures(driver_entity);
#endif
		}
	}

	return true;
}
