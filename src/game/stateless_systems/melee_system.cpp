#include "melee_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"

#include "game/messages/intent_message.h"
#include "game/messages/melee_swing_response.h"

#include "game/components/melee_component.h"
#include "game/components/melee_fighter_component.h"
#include "game/components/missile_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/torso_component.hpp"

#include "game/detail/frame_calculation.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/melee/like_melee.h"
#include "game/assets/animation_math.h"
#include "game/detail/inventory/perform_transfer.h"

using namespace augs;

namespace components {
	bool melee_fighter::now_returning() const {
		return state == melee_fighter_state::RETURNING || state == melee_fighter_state::CLASH_RETURNING;
	}

	bool melee_fighter::fighter_orientation_frozen() const {
		const bool allow_rotation = state == melee_fighter_state::READY || state == melee_fighter_state::COOLDOWN;
		return !allow_rotation;
	}
}

void melee_system::advance_thrown_melee_logic(const logic_step step) {
	auto& cosm = step.get_cosmos();

	cosm.for_each_having<components::melee>([&](const auto& it) {
		auto& sender = it.template get<components::sender>();

		if (sender.is_set()) {
			if (!has_hurting_velocity(it)) {
				sender.unset();
				it.infer_rigid_body();
				it.infer_colliders();
			}
		}
	});
}

const auto reset_cooldown_v = real32(-1.f);

void melee_system::initiate_and_update_moves(const logic_step step) {
	auto& cosm = step.get_cosmos();
	const auto& dt = step.get_delta();
	const auto anims = cosm.get_logical_assets().plain_animations;

	cosm.for_each_having<components::melee_fighter>([&](const auto& it) {
		const auto& fighter_def = it.template get<invariants::melee_fighter>();
		const auto& sentience = it.template get<components::sentience>();

		auto& fighter = it.template get<components::melee_fighter>();

		auto& state = fighter.state;
		auto& anim_state = fighter.anim_state;
		auto& elapsed_ms = anim_state.frame_elapsed_ms;

		auto reset_fighter = [&]() {
			/* Avoid cheating by quick weapon switches */
			state = melee_fighter_state::COOLDOWN;

			auto& cooldown_ms = elapsed_ms;
			cooldown_ms = reset_cooldown_v;
		};

		const auto wielded_items = it.get_wielded_items();

		if (wielded_items.size() != 1) {
			reset_fighter();
			return;
		}

		const auto target_weapon = cosm[wielded_items[0]];

		if (!target_weapon.template has<components::melee>()) {
			reset_fighter();
			return;
		}

		const auto& torso = it.template get<invariants::torso>();
		const auto& stance = torso.calc_stance(it, wielded_items);

		const auto chosen_action = [&]() {
			for (std::size_t i = 0; i < hand_count_v; ++i) {
				const auto& hand_flag = sentience.hand_flags[i];

				if (hand_flag) {
					const auto action = it.calc_hand_action(i);
					return action.type;
				}
			}

			return weapon_action_type::COUNT;
		}();

		const auto dt_ms = dt.in_milliseconds();

		target_weapon.template dispatch_on_having_all<components::melee>(
			[&](const auto& typed_weapon) {
				const auto& melee_def = typed_weapon.template get<invariants::melee>();

				if (state == melee_fighter_state::READY) {
					if (chosen_action == weapon_action_type::COUNT) {
						return;
					}

					state = melee_fighter_state::IN_ACTION;
					fighter.action = chosen_action;

					const auto& current_attack_def = melee_def.actions.at(chosen_action);

					const auto& body = it.template get<components::rigid_body>();
					const auto vel_dir = vec2(body.get_velocity()).normalize();

					if (const auto crosshair = it.find_crosshair()) {
						fighter.overridden_crosshair_base_offset = crosshair->base_offset;

						const auto cross_dir = vec2(crosshair->base_offset).normalize();

						const auto impulse_mult = (vel_dir.dot(cross_dir) + 1) / 2;

						const auto& movement_def  = it.template get<invariants::movement>();
						const auto conceptual_max_speed = std::max(1.f, movement_def.max_speed_for_animation);
						const auto current_velocity = body.get_velocity();
						const auto current_speed = current_velocity.length();

						const auto speed_mult = current_speed / conceptual_max_speed;

						{
							const auto min_effect = chosen_action == weapon_action_type::PRIMARY ? 0.8f : 0.f;
							const auto max_effect = chosen_action == weapon_action_type::PRIMARY ? 1.3f : 1.1f;

							const auto total_mult = std::min(max_effect, std::max(min_effect, speed_mult * impulse_mult));

							auto effect = current_attack_def.wielder_init_particles;
							effect.modifier.scale_amounts = total_mult;
							effect.modifier.scale_lifetimes = total_mult;

							effect.start(
								step,
								particle_effect_start_input::at_entity(it)
							);
						}

						body.apply_impulse(impulse_mult * cross_dir * body.get_mass() * current_attack_def.wielder_impulse);

						auto& movement = it.template get<components::movement>();
						movement.linear_inertia_ms += current_attack_def.wielder_inert_for_ms;

						{
							const auto min_effect = 0.88f;
							const auto max_effect = 1.f;

							const auto total_mult = std::min(max_effect, std::max(min_effect, speed_mult * impulse_mult));

							auto effect = current_attack_def.init_sound;
							effect.modifier.pitch = total_mult;

							effect.start(
								step,
								sound_effect_start_input::at_entity(typed_weapon).set_listener(it)
							);
						}

						{
							const auto& effect = current_attack_def.init_particles;

							effect.start(
								step,
								particle_effect_start_input::at_entity(typed_weapon)
							);
						}
					}

				}

				if (state == melee_fighter_state::COOLDOWN) {
					auto& cooldown_left_ms = elapsed_ms;

					if (cooldown_left_ms == reset_cooldown_v) {
						for (const auto& a : melee_def.actions) {
							if (cooldown_left_ms < a.cooldown_ms) {
								cooldown_left_ms = a.cooldown_ms;
							}
						}

						return;
					}

					const auto speed_mult = fighter_def.cooldown_speed_mult;

					cooldown_left_ms -= dt_ms * speed_mult;

					if (cooldown_left_ms <= 0.f) {
						state = melee_fighter_state::READY;
						cooldown_left_ms = 0.f;
					}

					return;
				}

				const auto prev_index = anim_state.frame_num;

				auto infer_if_different_frames = [&]() {
					const auto next_index = anim_state.frame_num;

					if (next_index != prev_index) {
						typed_weapon.infer_colliders_from_scratch();

						return true;
					}

					return false;
				};

				const auto& action = fighter.action;
				const auto& stance_anims = stance.actions[action];

				if (state == melee_fighter_state::IN_ACTION) {
					if (const auto* const current_anim = mapped_or_nullptr(anims, stance_anims.perform)) {
						const auto& f = current_anim->frames;

						if (!anim_state.advance(dt_ms, f)) {
							/* Animation is in progress. */
							infer_if_different_frames();
						}
						else {
							/* The animation has finished. */
							state = melee_fighter_state::RETURNING;
							anim_state.frame_num = 0;
						}
					}

					return;
				}

				if (fighter.now_returning()) {
					if (const auto* const current_anim = mapped_or_nullptr(anims, stance_anims.returner)) {
						const auto& f = current_anim->frames;

						if (!anim_state.advance(dt_ms, f)) {
							/* Animation is in progress. */
							infer_if_different_frames();
						}
						else {
							/* The animation has finished. */
							state = melee_fighter_state::COOLDOWN;
							anim_state.frame_num = 0;

							const auto total_ms = ::calc_total_duration(f);

							const auto& current_attack_def = melee_def.actions.at(action);

							if (action == weapon_action_type::PRIMARY) {
								const auto new_hand = it.get_first_free_hand();
								auto request = item_slot_transfer_request::standard(typed_weapon, new_hand);
								request.params.perform_recoils = false;

								perform_transfer_no_step(request, cosm);
							}
							else {
								typed_weapon.infer_colliders_from_scratch();
							}

							auto& cooldown_left_ms = elapsed_ms;
							cooldown_left_ms = std::max(0.f, current_attack_def.cooldown_ms - total_ms);
						}
					}

					return;
				}

			}
		);
	});
}