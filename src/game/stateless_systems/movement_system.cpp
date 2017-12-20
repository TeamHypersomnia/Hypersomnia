#include "augs/math/vec2.h"
#include "movement_system.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/intent_message.h"
#include "game/messages/movement_event.h"
#include "augs/log.h"

#include "game/components/gun_component.h"

#include "game/components/rigid_body_component.h"
#include "game/components/movement_component.h"
#include "game/components/sentience_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/stateless_systems/sentience_system.h"

#include "game/detail/physics/physics_scripts.h"

using namespace augs;

void movement_system::set_movement_flags_from_input(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& delta = step.get_delta();
	const auto& events = step.transient.messages.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		cosmos(
			it.subject,
			[&](const auto subject) {
				auto* const movement = subject.template find<components::movement>();

				if (movement == nullptr) {
					return;
				}

				switch (it.intent) {
				case game_intent_type::MOVE_FORWARD:
					movement->moving_forward = it.was_pressed();
					break;
				case game_intent_type::MOVE_BACKWARD:
					movement->moving_backward = it.was_pressed();
					break;
				case game_intent_type::MOVE_LEFT:
					movement->moving_left = it.was_pressed();
					break;
				case game_intent_type::MOVE_RIGHT:
					movement->moving_right = it.was_pressed();
					break;
				case game_intent_type::WALK:
					movement->walking_enabled = it.was_pressed();
					break;
				case game_intent_type::SPRINT:
					movement->sprint_enabled = it.was_pressed();
					break;
				default: break;
				}
			}
		);
	}
}

void movement_system::apply_movement_forces(cosmos& cosmos) {
	auto& physics_sys = cosmos.inferred.physics;
	const auto& delta = cosmos.get_fixed_delta();

	cosmos.for_each(
		processing_subjects::WITH_MOVEMENT,
		[&](const entity_handle it) {
			auto& movement = it.get<components::movement>();

			const auto& rigid_body = it.get<components::rigid_body>();

			if (!rigid_body.is_constructed()) {
				return;
			}

			auto movement_force_mult = 1.f;

			bool is_sprint_effective = movement.sprint_enabled;

			auto* const sentience = it.find<components::sentience>();
			const bool is_sentient = sentience != nullptr;

			value_meter::damage_result consciousness_damage_by_sprint;
			float minimum_consciousness_to_sprint = 0.f;

			if (is_sentient) {
				auto& consciousness = sentience->get<consciousness_meter_instance>();

				minimum_consciousness_to_sprint = consciousness.get_maximum_value() / 10;

				if (consciousness.value < minimum_consciousness_to_sprint - 0.1f) {
					movement_force_mult /= 2;
				}

				consciousness_damage_by_sprint = consciousness.calculate_damage_result(
					2 * delta.in_seconds(),
					minimum_consciousness_to_sprint
				);

				if (consciousness_damage_by_sprint.excessive > 0) {
					is_sprint_effective = false;
				}

				const auto& haste = sentience->get<haste_perk_instance>();

				if (haste.timing.is_enabled(cosmos.get_timestamp(), cosmos.get_fixed_delta())) {
					if (haste.is_greater) {
						movement_force_mult *= 1.45f;
					}
					else {
						movement_force_mult *= 1.3f;
					}
				}
			}

			const bool is_inert = movement.make_inert_for_ms > 0.f;

			if (is_inert) {
				movement.make_inert_for_ms -= static_cast<float>(delta.in_milliseconds());
			}

			const auto requested_by_input = movement.get_force_requested_by_input();

			if (requested_by_input.non_zero()) {
				if (is_sprint_effective) {
					movement_force_mult /= 2.f;

					if (is_sentient) {
						sentience->get<consciousness_meter_instance>().value -= consciousness_damage_by_sprint.effective;
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

				auto applied_force = requested_by_input;

				if (movement.acceleration_length > 0) {
					applied_force.set_length(movement.acceleration_length);
				}

				applied_force *= movement_force_mult;
				applied_force *= rigid_body.get_mass();

				rigid_body.apply_force(
					applied_force, 
					movement.applied_force_offset
				);
			}
			
			resolve_dampings_of_body(it, is_sprint_effective);
		}
	);
}

void movement_system::generate_movement_events(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& delta = step.get_delta();

	cosmos.for_each(
		processing_subjects::WITH_MOVEMENT,
		[&](const entity_handle it) {
			const auto& movement = it.get<components::movement>();

			float32 speed = 0.0f;

			if (movement.enable_animation) {
				if (it.has<components::rigid_body>()) {
					speed = it.get<components::rigid_body>().velocity().length();
				}
			}

			messages::movement_event msg;

			if (movement.max_speed_for_movement_event == 0.f) msg.speed = 0.f;
			else msg.speed = speed / movement.max_speed_for_movement_event;
			
			for (const auto receiver : movement.response_receivers) {
				messages::movement_event copy(msg);
				copy.stop_response_at_zero_speed = receiver.stop_response_at_zero_speed;
				copy.subject = receiver.target;
				step.post_message(copy);
			}
		}
	);
}
