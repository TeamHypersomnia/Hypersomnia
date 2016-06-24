#include "driver_system.h"
#include "game/messages/trigger_hit_confirmation_message.h"
#include "game/messages/collision_message.h"
#include "game/messages/intent_message.h"

#include "game/components/trigger_component.h"
#include "game/components/car_component.h"
#include "game/components/input_receiver_component.h"
#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/physics_component.h"
#include "game/components/force_joint_component.h"

#include "game/cosmos.h"
#include "game/stateful_systems/physics_system.h"

#include "game/components/driver_component.h"
#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

#include "game/entity_handle.h"
#include "game/step.h"

void driver_system::assign_drivers_from_successful_trigger_hits(fixed_step& step) {
	auto& confirmations = step.messages.get_queue<messages::trigger_hit_confirmation_message>();

	for (auto& e : confirmations) {
		auto subject_car = cosmos[e.trigger).get<components::trigger>(].entity_to_be_notified;

		if (cosmos[subject_car].dead())
			continue;

		auto* maybe_car = cosmos[subject_car].find<components::car>();

		if (maybe_car && e.trigger == maybe_car->left_wheel_trigger)
			assign_car_ownership(cosmos[e.detector_body), cosmos.get_handle(subject_car)];
	}
}

void driver_system::release_drivers_due_to_ending_contact_with_wheel(fixed_step& step) {
	auto& contacts = step.messages.get_queue<messages::collision_message>();
	auto& physics = cosmos.stateful_systems.get<physics_system>();

	for (auto& c : contacts) {
		if (c.type == messages::collision_message::event_type::END_CONTACT) {
			auto driver = cosmos[c.subject];
			auto car = physics.get_owner_body_entity(c.collider);

			auto* maybe_driver = cosmos[driver].find<components::driver>();

			if (maybe_driver) {
				if (maybe_driver->owned_vehicle == car) {
					release_car_ownership(driver);
					cosmos[driver).get<components::movement>(].make_inert_for_ms = 500.f;
				}
			}
		}
	}
}
void driver_system::release_drivers_due_to_requests(fixed_step& step) {
	auto& intents = step.messages.get_queue<messages::intent_message>();

	for (auto& e : intents)
		if (e.intent == intent_type::RELEASE_CAR && e.pressed_flag)
			release_car_ownership(cosmos[e.subject)];
}

bool driver_system::release_car_ownership(entity_handle driver) {
	return change_car_ownership(driver, driver.get_cosmos().get_handle(entity_id()), true);
}

bool driver_system::assign_car_ownership(entity_handle driver, entity_handle car) {
	return change_car_ownership(driver, car, false);
}

bool driver_system::change_car_ownership(entity_handle driver_entity, entity_handle car_entity, bool lost_ownership) {
	auto& driver = driver_entity.get<components::driver>();
	auto& cosmos = driver_entity.get_cosmos();
	auto& physics = cosmos.stateful_systems.get<physics_system>();

	auto* maybe_rotation_copying = driver_entity.find<components::rotation_copying>();
	auto* maybe_physics = driver_entity.find<components::physics>();
	auto* maybe_movement = driver_entity.find<components::movement>();
	auto& force_joint = driver_entity.get<components::force_joint>();

	if (!lost_ownership) {
		auto& car = car_entity.get<components::car>();

		if (cosmos[car.current_driver].alive())
			return false;

		driver.owned_vehicle = car_entity;
		car.current_driver = driver_entity;
		force_joint.chased_entity = car.left_wheel_trigger;
		driver_entity.unskip_processing_in(processing_subjects::WITH_FORCE_JOINT);

		if (maybe_movement) {
			maybe_movement->reset_movement_flags();
			maybe_movement->enable_braking_damping = false;
			maybe_movement->enable_animation = false;
		}

		if (maybe_rotation_copying && maybe_physics) {
			maybe_rotation_copying->update_value = false;
		}

		if (maybe_physics) {
			maybe_physics->set_transform(car.left_wheel_trigger);
			maybe_physics->set_velocity(vec2(0, 0));
			physics.resolve_density_of_associated_fixtures(driver_entity);
		}
	}
	else {
		auto& car = cosmos[driver.owned_vehicle].get<components::car>();

		driver.owned_vehicle.unset();
		car.current_driver.unset();
		driver_entity.skip_processing_in(processing_subjects::WITH_FORCE_JOINT);

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
		}

		if (maybe_physics) {
			physics.resolve_density_of_associated_fixtures(driver_entity);
		}
	}

	// networking the message, since it was successful

	return true;
}
