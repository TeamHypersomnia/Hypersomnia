#include "math/vec2.h"
#include "movement_system.h"
#include "game/cosmos.h"
#include "game/messages/intent_message.h"
#include "game/messages/movement_response.h"
#include "log.h"

#include "game/components/gun_component.h"

#include "game/components/physics_component.h"
#include "game/components/movement_component.h"

#include "game/entity_handle.h"
#include "game/step_state.h"

using namespace augs;


void movement_system::set_movement_flags_from_input(cosmos& cosmos, step_state& step) {
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
	auto& physics_sys = cosmos.stateful_systems.get<physics_system>();

	auto targets = cosmos.get(processing_subjects::WITH_MOVEMENT);
	for (auto it : targets) {
		auto& movement = it.get<components::movement>();

		if (!movement.apply_movement_forces) continue;

		auto* maybe_physics = it.find<components::physics>();

		vec2 resultant;

		resultant.x = movement.moving_right * movement.input_acceleration_axes.x - movement.moving_left * movement.input_acceleration_axes.x;
		resultant.y = movement.moving_backward * movement.input_acceleration_axes.y - movement.moving_forward * movement.input_acceleration_axes.y;

		if (maybe_physics == nullptr) {
			it.get<components::transform>().pos += resultant * cosmos.delta.in_seconds();
			continue;
		}

		auto& physics = *maybe_physics;
		
		if (movement.make_inert_for_ms > 0.f) {
			movement.make_inert_for_ms -= cosmos.delta.in_milliseconds();
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

			physics.apply_force(resultant * physics.body->GetMass(), movement.applied_force_offset, true);
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

void movement_system::generate_movement_responses(cosmos& cosmos, step_state& step) {
	step.messages.get_queue<movement_response>().clear();

	auto targets = cosmos.get(processing_subjects::WITH_MOVEMENT);
	for (auto it : targets) {
		auto& movement = it.get<components::movement>();

		auto* maybe_physics = it.find<components::physics>();
		
		float32 speed = 0.0f;

		if (movement.enable_animation) {
			if (maybe_physics == nullptr) {
				if (it.get<components::render>().interpolation_direction().non_zero())
					speed = movement.max_speed_for_movement_response;
			}
			else
				speed = maybe_physics->velocity().length();
		}

		movement_response msg;

		if (movement.max_speed_for_movement_response == 0.f) msg.speed = 0.f;
		else msg.speed = speed / movement.max_speed_for_movement_response;
		
		for (auto receiver : movement.response_receivers) {
			movement_response copy(msg);
			copy.stop_response_at_zero_speed = receiver.stop_response_at_zero_speed;
			copy.subject = receiver.target;
			step.messages.post(copy);
		}
	}
}
