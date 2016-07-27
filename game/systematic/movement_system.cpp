#include "augs/math/vec2.h"
#include "movement_system.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/intent_message.h"
#include "game/messages/movement_response.h"
#include "augs/log.h"

#include "game/components/gun_component.h"

#include "game/components/physics_component.h"
#include "game/components/movement_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

using namespace augs;

void movement_system::set_movement_flags_from_input(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	auto events = step.messages.get_queue<messages::intent_message>();

	for (auto it : events) {
		auto* movement = cosmos[it.subject].find<components::movement>();
		if (movement == nullptr) continue;

		switch (it.intent) {
		case intent_type::MOVE_FORWARD:
			movement->moving_forward = it.pressed_flag;
			break;
		case intent_type::MOVE_BACKWARD:
			movement->moving_backward = it.pressed_flag;
			break;
		case intent_type::MOVE_LEFT:
			movement->moving_left = it.pressed_flag;
			break;
		case intent_type::MOVE_RIGHT:
			movement->moving_right = it.pressed_flag;
			break;
		case intent_type::WALK:
			movement->walking_enabled = it.pressed_flag;
			break;
		default: break;
		}
	}
}

void movement_system::apply_movement_forces(cosmos& cosmos) {
	auto& physics_sys = cosmos.temporary_systems.get<physics_system>();
	auto delta = cosmos.significant.delta;

	auto targets = cosmos.get(processing_subjects::WITH_MOVEMENT);
	for (auto it : targets) {
		auto& movement = it.get<components::movement>();

		if (!movement.apply_movement_forces) continue;

		vec2 resultant;

		resultant.x = movement.moving_right * movement.input_acceleration_axes.x - movement.moving_left * movement.input_acceleration_axes.x;
		resultant.y = movement.moving_backward * movement.input_acceleration_axes.y - movement.moving_forward * movement.input_acceleration_axes.y;

		if (!it.has<components::physics>()) {
			it.get<components::transform>().pos += resultant * delta.in_seconds();
			continue;
		}

		auto& physics = it.get<components::physics>();
		
		if (!physics.is_constructed())
			continue;

		if (movement.make_inert_for_ms > 0.f) {
			movement.make_inert_for_ms -= static_cast<float>(delta.in_milliseconds());
			physics.set_linear_damping(2);
		}
		else
			physics.set_linear_damping(movement.standard_linear_damping);

		if (resultant.non_zero()) {
			if (movement.acceleration_length > 0)
				resultant.set_length(movement.acceleration_length);
			
			if (movement.make_inert_for_ms > 0.f)
				resultant /= 10.f;

			if (movement.walking_enabled)
				resultant /= 2.f;

			physics.apply_force(resultant * physics.get_mass(), movement.applied_force_offset, true);
		}

		/* the player feels less like a physical projectile if we brake per-axis */
		if (movement.enable_braking_damping && !(movement.make_inert_for_ms > 0.f))
			physics.set_linear_damping_vec(vec2(
				resultant.x_non_zero() ? 0.f : movement.braking_damping,
				resultant.y_non_zero() ? 0.f : movement.braking_damping));
		else
			physics.set_linear_damping_vec(vec2(0,0));
	}
}

void movement_system::generate_movement_responses(fixed_step& step) {
	auto& cosmos = step.cosm;
	auto& delta = step.get_delta();
	step.messages.get_queue<messages::movement_response>().clear();

	auto targets = cosmos.get(processing_subjects::WITH_MOVEMENT);
	for (auto it : targets) {
		auto& movement = it.get<components::movement>();

		float32 speed = 0.0f;

		if (movement.enable_animation) {
			if (it.has<components::physics>()) {
				speed = it.get<components::physics>().velocity().length();
			}
			else {
				if (it.get<components::transform>().interpolation_direction().non_zero())
					speed = movement.max_speed_for_movement_response;
			}
		}

		messages::movement_response msg;

		if (movement.max_speed_for_movement_response == 0.f) msg.speed = 0.f;
		else msg.speed = speed / movement.max_speed_for_movement_response;
		
		for (auto receiver : movement.response_receivers) {
			messages::movement_response copy(msg);
			copy.stop_response_at_zero_speed = receiver.stop_response_at_zero_speed;
			copy.subject = receiver.target;
			step.messages.post(copy);
		}
	}
}
