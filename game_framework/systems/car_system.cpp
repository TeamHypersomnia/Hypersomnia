#include "car_system.h"
#include "../messages/trigger_hit_confirmation_message.h"
#include "../messages/car_ownership_change_message.h"
#include "../messages/intent_message.h"

#include "entity_system/world.h"

#include "../components/trigger_component.h"

void car_system::set_steering_flags_from_intents() {
	auto& intents = parent_world.get_message_queue<messages::intent_message>();

	for (auto& it : intents) {
		auto* maybe_car = it.subject->find<components::car>();
		if (maybe_car == nullptr) continue;

		auto& car = *maybe_car;

		switch (it.intent) {
		case messages::intent_message::intent_type::MOVE_FORWARD:
			car.accelerating = it.pressed_flag;
			break;
		case messages::intent_message::intent_type::MOVE_BACKWARD:
			car.deccelerating = it.pressed_flag;
			break;
		case messages::intent_message::intent_type::MOVE_LEFT:
			car.turning_left = it.pressed_flag;
			break;
		case messages::intent_message::intent_type::MOVE_RIGHT:
			car.turning_right = it.pressed_flag;
			break;
		default: break;
		}
	}
}

void car_system::apply_movement_forces() {
	for (auto it : targets) {
		auto& car = it->get<components::car>();
		auto& physics = it->get<components::physics>();

		vec2 resultant;

		resultant.x = car.turning_right * car.input_acceleration.x - car.turning_left * car.input_acceleration.x;
		resultant.y = car.deccelerating * car.input_acceleration.y - car.accelerating * car.input_acceleration.y;

		auto vel = physics.velocity();

		if (resultant.non_zero()) {
			auto len = resultant.length();
			physics.apply_force(resultant * physics.get_mass());
		}

		if (car.braking_damping >= 0.f) {
			physics.set_linear_damping_vec(vec2(
				resultant.x_non_zero() ? 0.f : car.braking_damping,
				resultant.y_non_zero() ? 0.f : car.braking_damping));
		}

		float speed = vel.length();
		vel.normalize_hint(speed);

		if ((vel.x != 0.f || vel.y != 0.f) && physics.air_resistance > 0.f)
			physics.apply_force(-vel * physics.air_resistance * speed);
	}
}