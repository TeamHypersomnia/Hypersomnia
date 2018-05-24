#include "augs/math/vec2.h"
#include "movement_system.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/intent_message.h"

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
	const auto& events = step.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		cosmos(
			it.subject,
			[&](const auto subject) {
				if (auto* const movement = subject.template find<components::movement>()) {
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
			}
		);
	}
}

void movement_system::apply_movement_forces(cosmos& cosmos) {
	const auto delta = cosmos.get_fixed_delta();

	cosmos.for_each_having<components::movement>(
		[&](const auto it) {
			auto& movement = it.template get<components::movement>();
			auto& movement_def = it.template get<invariants::movement>();

			const auto& rigid_body = it.template get<components::rigid_body>();

			if (!rigid_body.is_constructed()) {
				return;
			}

			components::sentience* const sentience = it.template find<components::sentience>();
			const bool is_sentient = sentience != nullptr;

			if (it.sentient_and_unconscious()) {
				/* Behave as if all input was unset */
				movement.reset_movement_flags();
			}

			auto movement_force_mult = 1.f;

			movement.was_sprint_effective = movement.sprint_enabled;

			value_meter::damage_result consciousness_damage_by_sprint;
			float minimum_consciousness_to_sprint = 0.f;

			if (is_sentient) {
				auto& consciousness = sentience->get<consciousness_meter_instance>();

				minimum_consciousness_to_sprint = consciousness.get_maximum_value() / 10;

				if (consciousness.value < minimum_consciousness_to_sprint - 0.1f) {
					movement_force_mult /= 2;
				}

				consciousness_damage_by_sprint = consciousness.calc_damage_result(
					2 * delta.in_seconds(),
					minimum_consciousness_to_sprint
				);

				if (consciousness_damage_by_sprint.excessive > 0) {
					movement.was_sprint_effective = false;
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

			const auto requested_by_input = movement.get_force_requested_by_input(movement_def.input_acceleration_axes);

			if (requested_by_input.non_zero()) {
				if (movement.was_sprint_effective) {
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

				if (movement_def.acceleration_length > 0) {
					applied_force.set_length(movement_def.acceleration_length);
				}

				applied_force *= movement_force_mult;
				applied_force *= rigid_body.get_mass();

				rigid_body.apply_force(
					applied_force, 
					movement_def.applied_force_offset
				);
			}

			const auto max_speed = std::max(1.f, movement_def.max_speed_for_animation);
			const auto animation_dt = delta.in_seconds() * rigid_body.get_velocity().length() / max_speed;

			movement.animation_amount += animation_dt;

			rigid_body.infer_caches();
		}
	);
}
