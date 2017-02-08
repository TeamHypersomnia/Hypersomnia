#include "augs/math/vec2.h"
#include "movement_system.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/intent_message.h"
#include "game/messages/movement_response.h"
#include "augs/log.h"

#include "game/components/gun_component.h"

#include "game/components/physics_component.h"
#include "game/components/movement_component.h"
#include "game/components/sentience_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/step.h"

#include "game/systems_stateless/sentience_system.h"

using namespace augs;

void movement_system::set_movement_flags_from_input(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	const auto& events = step.transient.messages.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		auto* const movement = cosmos[it.subject].find<components::movement>();
		
		if (movement == nullptr) {
			continue;
		}

		switch (it.intent) {
		case intent_type::MOVE_FORWARD:
			movement->moving_forward = it.is_pressed;
			break;
		case intent_type::MOVE_BACKWARD:
			movement->moving_backward = it.is_pressed;
			break;
		case intent_type::MOVE_LEFT:
			movement->moving_left = it.is_pressed;
			break;
		case intent_type::MOVE_RIGHT:
			movement->moving_right = it.is_pressed;
			break;
		case intent_type::WALK:
			movement->walking_enabled = it.is_pressed;
			break;
		case intent_type::SPRINT:
			movement->sprint_enabled = it.is_pressed;
			break;
		default: break;
		}
	}
}

void movement_system::apply_movement_forces(cosmos& cosmos) {
	auto& physics_sys = cosmos.systems_temporary.get<physics_system>();
	const auto& delta = cosmos.get_fixed_delta();

	for (const auto& it : cosmos.get(processing_subjects::WITH_MOVEMENT)) {
		auto& movement = it.get<components::movement>();

		if (!movement.apply_movement_forces) {
			continue;
		}

		vec2 resultant;

		resultant.x = movement.moving_right * movement.input_acceleration_axes.x - movement.moving_left * movement.input_acceleration_axes.x;
		resultant.y = movement.moving_backward * movement.input_acceleration_axes.y - movement.moving_forward * movement.input_acceleration_axes.y;

		if (!it.has<components::physics>()) {
			it.get<components::transform>().pos += resultant * delta.in_seconds();
			continue;
		}

		const auto& physics = it.get<components::physics>();

		if (!physics.is_constructed()) {
			continue;
		}

		auto movement_force_mult = 1.f;
		auto considered_damping = 0.f;

		bool is_sprint_effective = movement.sprint_enabled;

		auto* const sentience = it.find<components::sentience>();
		const bool is_sentient = sentience != nullptr;

		if (is_sentient) {
			if (sentience->consciousness.value <= 0.f) {
				is_sprint_effective = false;
			}

			if (sentience->haste.is_enabled()) {
				if (sentience->haste.is_greater) {
					movement_force_mult *= 1.4f;
				}
				else {
					movement_force_mult *= 1.2f;
				}
			}
		}

		const bool is_inert = movement.make_inert_for_ms > 0.f;

		if (is_inert) {
			movement.make_inert_for_ms -= static_cast<float>(delta.in_milliseconds());
			considered_damping = 2;
		}
		else {
			considered_damping = movement.standard_linear_damping;
		}

		if (resultant.non_zero()) {
			if (is_sprint_effective) {
				if (!is_inert) {
					considered_damping /= 4;
				}

				movement_force_mult /= 2.f;

				if (is_sentient) {
					sentience->consciousness.value -= sentience->consciousness.calculate_damage_result(2 * delta.in_seconds()).effective;
				}
			}

			if (movement.walking_enabled) {
				movement_force_mult /= 2.f;
			}

			if (is_inert) {
				movement_force_mult /= 10.f;
			}

			if (is_sentient) {
				sentience->time_of_last_exertion = cosmos.get_timestamp();
			}

			if (movement.acceleration_length > 0) {
				resultant.set_length(movement.acceleration_length);
			}

			resultant *= movement_force_mult;

			physics.apply_force(resultant * physics.get_mass(), movement.applied_force_offset, true);
		}
		
		physics.set_linear_damping(considered_damping);

		/* the player feels less like a physical projectile if we brake per-axis */
		if (movement.enable_braking_damping && !(movement.make_inert_for_ms > 0.f)) {
			physics.set_linear_damping_vec(vec2(
				resultant.x_non_zero() ? 0.f : movement.braking_damping,
				resultant.y_non_zero() ? 0.f : movement.braking_damping));
		}
		else {
			physics.set_linear_damping_vec(vec2(0, 0));
		}
	}
}

void movement_system::generate_movement_responses(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	step.transient.messages.get_queue<messages::movement_response>().clear();

	for (const auto& it : cosmos.get(processing_subjects::WITH_MOVEMENT)) {
		const auto& movement = it.get<components::movement>();

		float32 speed = 0.0f;

		if (movement.enable_animation) {
			if (it.has<components::physics>()) {
				speed = it.get<components::physics>().velocity().length();
			}
		}

		messages::movement_response msg;

		if (movement.max_speed_for_movement_response == 0.f) msg.speed = 0.f;
		else msg.speed = speed / movement.max_speed_for_movement_response;
		
		for (const auto receiver : movement.response_receivers) {
			messages::movement_response copy(msg);
			copy.stop_response_at_zero_speed = receiver.stop_response_at_zero_speed;
			copy.subject = receiver.target;
			step.transient.messages.post(copy);
		}
	}
}
