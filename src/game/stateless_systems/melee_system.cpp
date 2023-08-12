#include "melee_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"

#include "game/messages/intent_message.h"

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
#include "augs/math/convex_hull.h"
#include "game/enums/filters.h"
#include "game/detail/physics/physics_queries.h"
#include "augs/misc/enum/enum_bitset.h"
#include "game/messages/damage_message.h"
#include "game/messages/thunder_effect.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/detail/movement/dash_logic.h"
#include "game/detail/movement/movement_getters.h"
#include "game/detail/organisms/startle_nearbly_organisms.h"
#include "game/detail/missile/headshot_detection.hpp"

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

	std::vector<vec2> total_verts;
	const auto si = cosm.get_si();
	const auto& physics = cosm.get_solvable_inferred().physics;

	cosm.for_each_having<components::melee_fighter>([&](const auto& it) {
		const auto& fighter_def = it.template get<invariants::melee_fighter>();
		auto& sentience = it.template get<components::sentience>();

		auto& fighter = it.template get<components::melee_fighter>();

		fighter.throw_cooldown_ms = std::max(-1.f, fighter.throw_cooldown_ms - dt.in_milliseconds());

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
		const bool consider_weapon_reloading = true;
		const auto& stance = torso.calc_stance(it, wielded_items, consider_weapon_reloading);

		const auto chosen_action = [&]() {
			for (std::size_t i = 0; i < hand_count_v; ++i) {
				if (::get_hand_flag(it, i)) {
					const auto action = it.calc_viable_hand_action(i);
					return action.type;
				}
			}

			return weapon_action_type::COUNT;
		}();

		const auto dt_ms = dt.in_milliseconds();

		target_weapon.template dispatch_on_having_all<components::melee>(
			[&](const auto& typed_weapon) {
				const auto& melee_def = typed_weapon.template get<invariants::melee>();
				auto& melee = typed_weapon.template get<components::melee>();

				if (state == melee_fighter_state::READY) {
					if (chosen_action == weapon_action_type::COUNT) {
						return;
					}

					const auto& current_attack_def = melee_def.actions.at(chosen_action);

					if (augs::is_positive_epsilon(current_attack_def.cp_required)) {
						auto& consciousness = sentience.template get<consciousness_meter_instance>();

						if (consciousness.value <= 0.0f) {
							return;
						}

						const auto consciousness_damage_by_attack = current_attack_def.cp_required;

						sentience.time_of_last_exertion = cosm.get_timestamp();
						consciousness.value -= consciousness_damage_by_attack;
					}

					state = melee_fighter_state::IN_ACTION;
					fighter.action = chosen_action;
					fighter.hit_obstacles.clear();
					fighter.hit_others.clear();
					fighter.previous_frame_transform = typed_weapon.get_logic_transform();

					typed_weapon.infer_colliders_from_scratch();

					if (const auto crosshair = it.find_crosshair()) {
						fighter.overridden_crosshair_base_offset = crosshair->base_offset;

						const auto effect_mult = 
							::dash_conditions_fulfilled(it) 
							? ::perform_dash_towards_crosshair(
								it,
								current_attack_def.wielder_impulse,
								current_attack_def.wielder_inert_for_ms,
								dash_flags()
							)
							: 0.f
						;

						if (effect_mult > 0.f) {
							const auto min_effect = chosen_action == weapon_action_type::PRIMARY ? 0.8f : 0.f;
							const auto max_effect = chosen_action == weapon_action_type::PRIMARY ? 1.3f : 1.1f;

							const auto chosen_mult = std::clamp(effect_mult, min_effect, max_effect);

							auto effect = current_attack_def.wielder_init_particles;
							effect.modifier *= chosen_mult;

							effect.start(
								step,
								particle_effect_start_input::at_entity(it),
								predictable_only_by(it)
							);
						}

						{
							const auto min_effect = 0.88f;
							const auto max_effect = 1.f;

							const auto chosen_mult = std::clamp(effect_mult, min_effect, max_effect);

							auto effect = current_attack_def.init_sound;
							effect.modifier.pitch *= chosen_mult;

							effect.start(
								step,
								sound_effect_start_input::at_entity(typed_weapon).set_listener(it),
								predictable_only_by(it)
							);
						}

						{
							const auto& effect = current_attack_def.init_particles;

							effect.start(
								step,
								particle_effect_start_input::at_entity(typed_weapon),
								predictable_only_by(it)
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

				auto detect_damage = [&](
					const transformr& from,
					const transformr& to
				) {
					if (fighter.now_returning()) {
						return;
					}

					const auto& current_attack_def = melee_def.actions.at(fighter.action);
					const auto& damage_def = current_attack_def.damage;

					const auto speed = it.get_effective_velocity().length();
					const auto bonus_mult = speed * current_attack_def.bonus_damage_speed_ratio;

					{
						const auto startle_mult = 0.8f + (1 + bonus_mult) * 3.5f;

						startle_nearby_organisms(
							cosm,
							to.pos,
							damage_def.base * startle_mult * 1.4f,
							damage_def.base * startle_mult,
							startle_type::IMMEDIATE,
							scare_source::MELEE
						);
					}

					const auto impact_velocity = (to.pos - from.pos) * dt.in_steps_per_second();

					const auto image_id = typed_weapon.get_image_id();
					const auto& offsets = cosm.get_logical_assets().get_offsets(image_id);

					if (const auto& shape = offsets.non_standard_shape; !shape.empty()) {
						total_verts.clear();

						for (auto h : shape.source_polygon) {
							h.mult(from);
							total_verts.emplace_back(h);
						}

						for (auto h : shape.source_polygon) {
							h.mult(to);
							total_verts.emplace_back(h);
						}

						const auto subject_id = entity_id(it.get_id());

						auto detect_on = [&](const auto& convex) {
							const auto shape = to_polygon_shape(convex, si);
							const auto filter = predefined_queries::melee_query();

							auto handle_intersection = [&](
								const b2Fixture& fix,
								const vec2,
								const vec2 point_b
							) {
								const auto& point_of_impact = point_b;

								const auto victim = cosm[get_entity_that_owns(fix)];
								const auto victim_id = victim.get_id();

								const bool is_self = 
									victim_id == subject_id
									|| victim.get_owning_transfer_capability() == subject_id
								;

								if (is_self) {
									return callback_result::CONTINUE;
								}

								const auto hit_filter = fix.GetFilterData();
								const auto bs = augs::enum_bitset<filter_category>(static_cast<unsigned long>(hit_filter.categoryBits));

								const bool is_solid_obstacle = 
									bs.test(filter_category::WALL)
									|| bs.test(filter_category::GLASS_OBSTACLE)
								;

								const auto victim_sentience = victim.template find<invariants::sentience>();
								const bool victim_sentient = victim_sentience != nullptr;

								auto& already_hit = [&]() -> auto& {
									if (victim_sentient || is_solid_obstacle) {
										return fighter.hit_obstacles;
									}

									return fighter.hit_others;
								}();

								if (already_hit.size() < already_hit.max_size()) {
									const bool is_yet_unaffected = !found_in(already_hit, victim_id);

									const auto& body = it.template get<components::rigid_body>();

									if (is_solid_obstacle && already_hit.size() > 0 && victim_id == already_hit[0]) {
										body.apply_impulse(fighter.first_separating_impulse);
										fighter.first_separating_impulse = {};
									}
									else if (is_yet_unaffected) {
										already_hit.emplace_back(victim_id);

										if (is_solid_obstacle && already_hit.size() == 1) {
											{
												auto& current = sentience.rotation_inertia_ms;
												const auto& bonus = current_attack_def.obstacle_hit_rotation_inertia_ms;

												current = std::max(current, bonus);
											}

											if (const auto crosshair = it.find_crosshair()) {
												const auto& kickback = current_attack_def.obstacle_hit_kickback_impulse;

												const auto point_dir = (from.pos - point_of_impact).normalize();

												const auto ray_a = from.pos;
												const auto ray_b = point_of_impact - point_dir * 50;

												const auto ray = physics.ray_cast_px(
													cosm.get_si(),
													ray_a,
													ray_b,
													predefined_queries::melee_solid_obstacle_query(),
													typed_weapon
												);

												if (ray.hit) {
													const auto& n = ray.normal;

													if (DEBUG_DRAWING.draw_melee_info) {
														DEBUG_PERSISTENT_LINES.emplace_back(
															cyan,
															ray_a,
															ray_b
														);

														DEBUG_PERSISTENT_LINES.emplace_back(
															yellow,
															ray.intersection,
															ray.intersection + n * 100
														);
													}

													{
														const auto inter_dir = (ray_a - ray.intersection).normalize();

														impulse_input in;
														in.angular = n.cross(inter_dir) * current_attack_def.obstacle_hit_recoil_mult;
														it.apply_crosshair_recoil(in);

														const auto parallel_mult = n.dot(inter_dir);
														const auto considered_mult = std::max(parallel_mult - 0.4f, 0.f);
														const auto total_impulse_mult = considered_mult * kickback * body.get_mass() / 3;
														const auto total_impulse = total_impulse_mult * n;

														const auto vel = body.get_velocity();
														const auto target_vel = vec2(vel).reflect(n);

														const auto velocity_conservation = 0.6f;
														body.set_velocity(target_vel * velocity_conservation);

														body.apply_impulse(total_impulse);
														fighter.first_separating_impulse = total_impulse;
													}
												}
											}

											auto& movement = it.template get<components::movement>();
											movement.linear_inertia_ms += current_attack_def.obstacle_hit_linear_inertia_ms;
										}

										bool clash_applied = false;

										{
											const auto now = cosm.get_timestamp();

											auto cooldown_passes = [&](augs::stepped_timestamp& stamp, const int cooldown = 2) {
												return !(now.step <= stamp.step + cooldown);
											};

											auto try_pass_cooldown = [&](augs::stepped_timestamp& stamp, const int cooldown = 2) {
												if (!cooldown_passes(stamp, cooldown)) {
													return false;
												}

												stamp = now;
												return true;
											};

											auto create_clash_thunders = [&](const auto& impact_dir) {
												auto msg = messages::thunder_effect(never_predictable_v);
												auto& th = msg.payload;

												th.delay_between_branches_ms = {10.f, 35.f};
												th.max_branch_lifetime_ms = {60.f, 85.f};
												th.branch_length = {30.f, 150.f};

												th.max_all_spawned_branches = 80;
												th.max_branch_children = 2;

												th.branch_angle_spread = 40.f;
												th.color = white;

												th.first_branch_root = transformr(point_of_impact, impact_dir.perpendicular_cw().degrees());
												step.post_message(msg);

												th.first_branch_root = transformr(point_of_impact, impact_dir.perpendicular_ccw().degrees());
												step.post_message(msg);
											};

											const auto& clash_def = current_attack_def.clash;

											const auto subject_owner_pos = it.get_logic_transform().pos;

											if (is_like_melee_in_action(victim) && try_pass_cooldown(melee.when_clashed, 5)) {
												clash_applied = true;

												const auto victim_owner = victim.get_owning_transfer_capability();
												const auto victim_owner_pos = victim_owner.get_logic_transform().pos;
												const auto impact_dir = (victim_owner_pos - subject_owner_pos).normalize();

												const auto& v_melee_def = victim.template get<invariants::melee>();
												auto& v_melee = victim.template get<components::melee>();
												const auto& v_fighter = victim_owner.template get<components::melee_fighter>();

												v_melee.when_clashed = now;

												const auto& v_attack = v_melee_def.actions[v_fighter.action];
												const auto& v_clash_def = v_attack.clash;

												{
													auto play_clash = [&](const auto& def) {
														def.sound.start(
															step,
															sound_effect_start_input::fire_and_forget(point_of_impact),
															never_predictable_v
														);
													};

													play_clash(clash_def);

													if (clash_def.sound.id != v_clash_def.sound.id) {
														play_clash(v_clash_def);
													}
												}

												const auto subject_impact = -v_clash_def.impulse * impact_dir;
												const auto victim_impact = clash_def.impulse * impact_dir;

												body.apply_impulse(subject_impact);
												victim.template get<components::rigid_body>().apply_impulse(victim_impact);

												{
													auto subject_shake = v_attack.damage.shake;
													auto victim_shake = current_attack_def.damage.shake;

													subject_shake *= 2;
													victim_shake *= 2;

													victim_shake.apply(now, victim_owner.template get<invariants::sentience>(), victim_owner.template get<components::sentience>());
													subject_shake.apply(now, it.template get<invariants::sentience>(), it.template get<components::sentience>());
												}

												{
													const auto subject_inertia = v_clash_def.victim_inert_for_ms;
													const auto victim_inertia = clash_def.victim_inert_for_ms;
													
													it.template get<components::movement>().linear_inertia_ms += subject_inertia;
													victim_owner.template get<components::movement>().linear_inertia_ms += victim_inertia;
												}

												create_clash_thunders(impact_dir);
											}

											if (is_like_thrown_melee(victim) && try_pass_cooldown(melee.when_clashed, 5)) {
												const auto victim_pos = victim.get_logic_transform().pos;
												const auto impact_dir = (victim_pos - subject_owner_pos).normalize();

												clash_applied = true;

												const auto& v_melee_def = victim.template get<invariants::melee>();
												const auto& v_attack = v_melee_def.throw_def;
												const auto& v_clash_def = v_attack.clash;

												{
													auto play_clash = [&](const auto& def) {
														def.sound.start(
															step,
															sound_effect_start_input::fire_and_forget(point_of_impact),
															never_predictable_v
														);
													};

													//const auto& throw_clash_def = melee_def.throw_def.clash;

													const auto subject_impact = -v_clash_def.impulse * impact_dir;
													
													const auto make_returned_boomerang_more_offensive = 1.1f;
													const auto victim_impact = (v_attack.boomerang_impulse.linear + clash_def.impulse + it.get_effective_velocity().length()) * make_returned_boomerang_more_offensive * impact_dir;

													body.apply_impulse(subject_impact);

													const auto& victim_body = victim.template get<components::rigid_body>();

													victim_body.set_transform({ victim_pos, impact_dir.degrees() });
													victim_body.set_velocity(victim_impact);

													play_clash(clash_def);

													if (clash_def.sound.id != v_clash_def.sound.id) {
														play_clash(v_clash_def);
													}
												}

												create_clash_thunders(impact_dir);

												victim.template get<components::sender>().set(it);
											}
										}

										if (!clash_applied) {
											messages::damage_message damage_msg;

											const auto mult = 1.f + bonus_mult;

											damage_msg.damage = damage_def;
											damage_msg.damage *= mult;

											damage_msg.type = adverse_element_type::FORCE;
											damage_msg.origin = damage_origin(typed_weapon);
											damage_msg.subject = victim;
											damage_msg.impact_velocity = impact_velocity;
											damage_msg.point_of_impact = point_of_impact;

											if (victim_sentient) {
												const auto impact_dir = vec2(impact_velocity).normalize();

												const auto missile_begin = point_of_impact;
												const auto missile_end = missile_begin + impact_dir * (to.pos - from.pos).length();

												const auto head_transform = ::calc_head_transform(victim);
												const auto head_radius = victim_sentience->head_hitbox_radius * current_attack_def.head_radius_multiplier;

												if (head_transform.has_value()) {
													const auto head_pos = head_transform->pos;

													if (DEBUG_DRAWING.draw_headshot_detection) {
														DEBUG_PERSISTENT_LINES.emplace_back(
															cyan,
															missile_begin,
															missile_end
														);

														DEBUG_PERSISTENT_LINES.emplace_back(
															red,
															head_pos,
															head_pos + vec2(0, head_radius)
														);

														DEBUG_PERSISTENT_LINES.emplace_back(
															red,
															head_pos,
															head_pos + vec2(head_radius, 0)
														);

														DEBUG_PERSISTENT_LINES.emplace_back(
															red,
															head_pos,
															head_pos + vec2(-head_radius, 0)
														);

														DEBUG_PERSISTENT_LINES.emplace_back(
															red,
															head_pos,
															head_pos + vec2(0, -head_radius)
														);
													}

													const bool detected_first = ::headshot_detected_finite_ray(
														missile_begin,
														missile_end,
														head_pos,
														head_radius
													);

													const bool detected_second = ::headshot_detected_finite_ray(
														missile_end,
														missile_begin,
														head_pos,
														head_radius
													);

													if (detected_first || detected_second) {
														damage_msg.head_transform = *head_transform;
														damage_msg.headshot_mult = current_attack_def.headshot_multiplier;
														damage_msg.origin.circumstances.headshot = true;
													}
												}
											}

											step.post_message(damage_msg);
										}
									}
								}

								return callback_result::CONTINUE;
							};

							physics.for_each_intersection_with_polygon(
								si,
								convex,
								filter,
								handle_intersection
							);
						};

						const auto max_v = b2_maxPolygonVertices;
						auto hull = augs::convex_hull(total_verts);

						/* Double-duty */
						auto& acceptable = total_verts;
						acceptable.clear();

						while (hull.size() > max_v) {
							acceptable.assign(hull.begin(), hull.begin() + max_v);
							hull.erase(hull.begin() + 1, hull.begin() + max_v - 1);

							detect_on(acceptable);
						}

						detect_on(hull);
					}
					else {
						ensure(false && "A knife is required to have a non-standard shape defined.");
					}
				};

				auto infer_if_different_frames = [&]() {
					const auto next_index = anim_state.frame_num;

					if (next_index != prev_index) {
						const auto damage_from = fighter.previous_frame_transform;
						typed_weapon.infer_colliders_from_scratch();
						const auto damage_to = typed_weapon.get_logic_transform();
						fighter.previous_frame_transform = damage_to;

						detect_damage(damage_from, damage_to);

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

							fighter.throw_cooldown_ms = std::max(fighter.throw_cooldown_ms, cooldown_left_ms);
						}
					}

					return;
				}

			}
		);
	});
}