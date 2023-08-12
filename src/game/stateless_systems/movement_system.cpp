#include "augs/math/vec2.h"
#include "movement_system.h"
#include "game/cosmos/cosmos.h"
#include "game/messages/intent_message.h"

#include "game/components/gun_component.h"

#include "game/components/rigid_body_component.h"
#include "game/components/movement_component.h"
#include "game/components/sentience_component.h"
#include "game/components/torso_component.hpp"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"

#include "game/stateless_systems/sentience_system.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/frame_calculation.h"
#include "game/detail/visible_entities.h"
#include "game/detail/sentience/sentience_getters.h"

#include "game/detail/physics/infer_damping.hpp"
#include "game/detail/movement/dash_logic.h"
#include "game/detail/movement/movement_getters.h"
#include "game/detail/sentience/tool_getters.h"
#include "game/detail/crosshair_math.hpp"
#include "game/detail/get_hovered_world_entity.h"

using namespace augs;

namespace augs {
	template <class T>
	bool flip_if_gt(T& value, const T& bound) {
		const auto diff = value - bound;

		if (diff > static_cast<T>(0)) {
			value = bound - diff;
			return true;
		}

		return false;
	}

	template <class T>
	bool flip_if_lt(T& value, const T& bound) {
		const auto diff = value - bound;

		if (diff < static_cast<T>(0)) {
			value = bound - diff;
			return true;
		}

		return false;
	}
}

void movement_system::set_movement_flags_from_input(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& events = step.get_queue<messages::intent_message>();

	for (const auto& it : events) {
		cosm(
			it.subject,
			[&](const auto subject) {
				if (auto* const movement = subject.template find<components::movement>()) {
					auto& flags = movement->flags;

					switch (it.intent) {
						case game_intent_type::MOVE_FORWARD:
							flags.forward = it.was_pressed();
							break;
						case game_intent_type::MOVE_BACKWARD:
							flags.backward = it.was_pressed();
							break;
						case game_intent_type::MOVE_LEFT:
							flags.left = it.was_pressed();
							break;
						case game_intent_type::MOVE_RIGHT:
							flags.right = it.was_pressed();
							break;
						case game_intent_type::WALK_SILENTLY:
							flags.walking = it.was_pressed();
							break;
						case game_intent_type::SPRINT:
							flags.sprinting = it.was_pressed();
							break;
						case game_intent_type::DASH:
							flags.dashing = it.was_pressed();
							break;

						default: break;
					}
				}
			}
		);
	}
}

void movement_system::apply_movement_forces(const logic_step step) {
	auto& cosm = step.get_cosmos();

	const auto& clk = cosm.get_clock();
	const auto delta = clk.dt;
	const auto delta_ms = delta.in_milliseconds();

	cosm.for_each_having<components::movement>(
		[&](const auto& it) {
			auto& movement = it.template get<components::movement>();
			const auto& movement_def = it.template get<invariants::movement>();

			const auto& rigid_body = it.template get<components::rigid_body>();

			if (!rigid_body.is_constructed()) {
				return;
			}

			components::sentience* const sentience = it.template find<components::sentience>();
			const bool is_sentient = sentience != nullptr;

			auto considered_flags = movement.flags;

			if (it.is_frozen()) {
				considered_flags = {};
			}

			auto movement_force_mult = 1.f;

			movement.was_sprint_effective = considered_flags.sprinting;

			auto cp_damage_by_sprint = 0.0f;

			enum class haste_type {
				NONE,
				NORMAL,
				GREATER
			};

			const bool is_walking = !movement.was_sprint_effective && [&]() {
				return considered_flags.walking;
			}();

			movement.was_walk_effective = is_walking;

			const auto current_haste = [&]() {
				if (is_walking) {
					return haste_type::NONE;
				}

				if (is_sentient) {
					const auto& haste = sentience->get<haste_perk_instance>();

					if (haste.timing.is_enabled(clk)) {
						if (haste.is_greater) {
							return haste_type::GREATER;
						}
						else {
							return haste_type::NORMAL;
						}
					}
				}

				return haste_type::NONE;
			}();

			const auto requested_by_input_aa = considered_flags.get_force_requested_by_input(movement_def.input_acceleration_axes);
			const bool non_zero_requested = requested_by_input_aa.is_nonzero();
			const auto requested_by_input = [&]() {
				if (!movement.keep_movement_forces_relative_to_crosshair) {
					return requested_by_input_aa;
				}

				const auto disp = ::calc_crosshair_displacement(it);

				auto total_vec = vec2::zero;

				{
					auto side_disp = disp;

					if (side_disp.y > 0) {
						side_disp = -side_disp;
					}

					const auto angle = side_disp.degrees();
					const auto side_component = vec2(requested_by_input_aa.x, 0.f);
					total_vec += vec2(side_component).rotate(angle + 90);
				}

				const auto forward_component = vec2(0.f, requested_by_input_aa.y);
				const auto angle = ::calc_crosshair_displacement(it).degrees();

				total_vec += vec2(forward_component).rotate(angle + 90);
				return total_vec;
			}();

			if (is_sentient) {
				const auto& def = it.template get<invariants::sentience>();

				auto& cp = sentience->get<consciousness_meter_instance>();

#if SLOW_DOWN_IF_CP_CRITICAL
				if (cp.value < AUGS_EPSILON<real32>) {
					movement_force_mult /= 2;
				}
#endif

				cp_damage_by_sprint = def.sprint_drains_cp_per_second * delta.in_seconds();

				if (cp.value <= 0.0f) {
					cp_damage_by_sprint = 0.0f;
					movement.was_sprint_effective = false;
				}

				if (current_haste == haste_type::GREATER) {
					movement_force_mult *= 1.45f;
				}
				else if (current_haste == haste_type::NORMAL) {
					movement_force_mult *= 1.3f;
				}

				if (considered_flags.dashing && cp.value > 0.0f) {
					if (::dash_conditions_fulfilled(it)) {
						if (const auto crosshair_offset = it.find_crosshair_offset()) {
							const auto cp_damage_by_dash = def.dash_drains_cp;

							const auto final_direction = 
								non_zero_requested
								? vec2(requested_by_input).normalize()
								: vec2(it.get_effective_velocity()).normalize()
							;

							const auto dash_effect_mult = ::perform_dash(
								it,
								final_direction,

								movement_def.dash_impulse,
								movement_def.dash_inert_ms,
								dash_flag::PROPORTIONAL_TO_CURRENT_SPEED
							);

							if (dash_effect_mult > 0.f) {
								::perform_dash_effects(
									step,
									it,
									dash_effect_mult,
									predictable_only_by(it)
								);

								cp.value -= cp_damage_by_dash;
								sentience->time_of_last_exertion = cosm.get_timestamp();
								movement.dash_cooldown_ms = movement_def.dash_cooldown_ms;
							}
						}
					}

					//movement.flags.dashing = false;
				}
			}

			if (movement.dash_cooldown_ms > 0.f) {
				movement.dash_cooldown_ms -= delta_ms;
			}

			const bool portal_inertia = movement.portal_inertia_ms > 0.f;
			const bool linear_inertia = movement.linear_inertia_ms > 0.f;
			const bool surface_slowdown = movement.surface_slowdown_ms > 0.f;
			const bool constant_inertia = movement.const_inertia_ms > 0.f;

			const auto num_frames = movement_def.animation_frame_num;
			const auto frame_ms = movement_def.animation_frame_ms;

			auto calculate_frame_idx = [&]() {
				auto idx = static_cast<unsigned>(movement.animation_amount / frame_ms);
				idx = std::min(idx, num_frames - 1);
				idx = std::max(idx, 0u);

				return idx;
			};

			const auto time_to_gain_speed_ms = 100.f;
			const bool should_decelerate_due_to_walk = is_walking && movement.animation_amount >= time_to_gain_speed_ms;
			const bool propelling = !should_decelerate_due_to_walk && non_zero_requested;

			if (propelling) {
				if (movement.was_sprint_effective) {
					if (portal_inertia) {
						if (is_sentient) {
							sentience->get<consciousness_meter_instance>().value -= cp_damage_by_sprint * 0.5f;
						}
					}
					else {
						movement_force_mult /= 2.f;

						if (is_sentient) {
							sentience->get<consciousness_meter_instance>().value -= cp_damage_by_sprint;
						}
					}
				}

				if (constant_inertia) {
					movement_force_mult *= movement_def.const_inertia_mult;
				}

				if (surface_slowdown) {
					movement_force_mult *= movement.surface_slowdown_ms / movement_def.surface_slowdown_max_ms;
				}

				if (linear_inertia) {
					const auto m = movement_def.max_linear_inertia_when_movement_possible;
					const auto inertia_mult = std::clamp(1.f - movement.linear_inertia_ms / m, 0.f, 1.f);
					movement_force_mult *= inertia_mult;
				}

				if (is_sentient) {
					const auto& sentience_def = it.template get<invariants::sentience>();
					const auto regen_mult = sentience_def.cp_regen_mult_when_moving;
					const bool count_walk_as_exertion = regen_mult == 0.f;

					if (count_walk_as_exertion || movement.was_sprint_effective) {
						sentience->time_of_last_exertion = cosm.get_timestamp();
					}
				}

				auto applied_force = requested_by_input;

				if (movement_def.acceleration_length > 0) {
					applied_force.set_length(movement_def.acceleration_length * ::get_movement_speed_mult(it));
				}

				applied_force *= movement_force_mult;

				rigid_body.apply_force(
					applied_force, 
					movement_def.applied_force_offset
				);
			}

			const auto duration_bound = static_cast<real32>(num_frames * frame_ms);

			const auto conceptual_max_speed = std::max(1.f, movement_def.max_speed_for_animation);
			const auto current_velocity = rigid_body.get_velocity();
			const auto current_speed = current_velocity.length();

			const auto speed_mult = current_speed / conceptual_max_speed;

			auto start_footstep_effect = [&]() {
				const auto transform = it.get_logic_transform();

				const auto velocity_degrees = current_velocity.degrees();
				auto effect_transform = transformr(transform.pos, velocity_degrees);

				const auto leg_anim = it.template get<invariants::torso>().calc_leg_anim(
					current_velocity,
					transform.rotation
				);

				const auto anim_id = leg_anim.id;

				const auto& logicals = cosm.get_logical_assets();

				if (const auto anim = logicals.find(anim_id)) {
					/* Offset the effect transform by leg offset, if the animation exists */
					const auto frame = ::get_frame_and_flip(movement.four_ways_animation, *anim);
					const auto& im_def = logicals.get_offsets(frame.frame.image_id);

					auto offset = im_def.legs.foot;

					if (frame.flip.vertically) {
						offset.y *= -1;
					}

					if (frame.flip.horizontally) {
						offset.x *= -1;
					}

					effect_transform *= transformr(offset);
				}

				const auto& common_assets = cosm.get_common_assets();
				auto standard_effect = common_assets.standard_footstep;
				auto chosen_effect = standard_effect;
				
				{
					/* Choose effect based on where the foot has landed */

					real32 chosen_speed_mult = 1.f;

					get_hovered_world_entity(
						cosm,
#if 0
						effect_transform.pos,
#else
						transform.pos,
#endif
						[&](const auto ground_id) {
							if (const auto ground_entity = cosm[ground_id]) {
								if (const auto ground = ground_entity.template find<invariants::ground>()) {
									if (ground->footstep_effect.is_enabled) {
										chosen_effect = ground->footstep_effect.value;
										chosen_speed_mult = ground->movement_speed_mult;
										return true;
									}
								}
							}

							return false;
						},
						render_layer_filter::whitelist(render_layer::GROUND)
					);

					const auto drag_mult = 1 - chosen_speed_mult;

					rigid_body.apply_impulse(
						(current_velocity * -1 * drag_mult) * movement_def.surface_drag_mult,
						movement_def.applied_force_offset
					);

					movement.surface_slowdown_ms += movement_def.surface_slowdown_unit_ms * drag_mult;
					movement.surface_slowdown_ms = std::min(movement.surface_slowdown_ms, movement_def.surface_slowdown_max_ms);
				}

				auto& sound = chosen_effect.sound;

				if (!sound.id.is_set()) {
					sound = standard_effect.sound;
				}

				const auto gain_mult = speed_mult / 2;
				const auto pitch_mult = std::min(1.7f, 1 + gain_mult);

				sound.modifier.gain *= gain_mult;
				sound.modifier.pitch *= pitch_mult;

				if (sound.modifier.max_distance < 0.f) {
					sound.modifier.max_distance = standard_effect.sound.modifier.max_distance;
				}

				if (sound.modifier.reference_distance < 0.f) {
					sound.modifier.reference_distance = standard_effect.sound.modifier.reference_distance;
				}

				const auto predictability = predictable_only_by(it);

				sound.start(
					step, 
					sound_effect_start_input::at_listener(it.get_id()),
					predictability
				);

				const auto scale = std::max(0.8f, speed_mult);

				{
					auto& particles = chosen_effect.particles;

					if (!particles.id.is_set()) {
						particles = standard_effect.particles;
					}

					particles.modifier.scale_amounts *= scale;
					particles.modifier.scale_lifetimes *= scale;

					particles.start(
						step, 
						particle_effect_start_input::fire_and_forget(effect_transform),
						predictability
					);
				}

				if (current_haste != haste_type::NONE) {
					auto particles = common_assets.haste_footstep_particles;

					particles.modifier.scale_amounts *= scale;
					particles.modifier.scale_lifetimes *= scale;
					particles.modifier.color = green;

					effect_transform.rotation += 180;

					particles.start(
						step, 
						particle_effect_start_input::fire_and_forget(effect_transform),
						predictability
					);
				}
			};

			const bool freeze_leg_frame = movement.portal_inertia_ms > 0.0f || ::legs_frozen(it);
			const auto animation_dt = freeze_leg_frame ? 0.f : delta_ms * speed_mult;

			auto& backward = movement.four_ways_animation.backward;
			auto& amount = movement.animation_amount;

			if (!propelling && current_speed <= conceptual_max_speed / 2) {
				/* Animation is finishing. */
				const auto decreasing_dt = delta_ms * std::max(repro::sqrt(repro::sqrt(speed_mult)), 0.2f);
				amount = std::max(0.f, amount - decreasing_dt);
			}
			else {
				if (backward) {
					amount -= animation_dt;

					if (augs::flip_if_lt(amount, 0.f)) {
						backward = false;

						auto& f = movement.four_ways_animation.flip;
						f = !f;
					}
				}
				else {
					amount += animation_dt;

					if (augs::flip_if_gt(amount, duration_bound)) {
						backward = true;
					}
				}
			}

			movement.four_ways_animation.base_frames_n = num_frames;

			auto& idx = movement.four_ways_animation.index;

			const auto old_idx = idx;
			idx = calculate_frame_idx();

			if (old_idx == num_frames - 2 && idx == num_frames - 1) {
				start_footstep_effect();
			}

			rigid_body.infer_damping();

			if (surface_slowdown) {
				movement.surface_slowdown_ms -= delta_ms;
			}

			if (!rigid_body.get_special().inside_portal.is_set()) {
				if (constant_inertia) {
					movement.const_inertia_ms = std::min(300.f, movement.const_inertia_ms);
					movement.const_inertia_ms -= delta_ms;
				}

				if (linear_inertia) {
					movement.linear_inertia_ms -= delta_ms;
				}

				movement.portal_inertia_ms -= delta_ms;
			}

			movement.portal_inertia_ms = std::clamp(movement.portal_inertia_ms, 0.0f, 3000.0f);
			movement.linear_inertia_ms = std::clamp(movement.linear_inertia_ms, 0.0f, 3000.0f);
		}
	);
}
