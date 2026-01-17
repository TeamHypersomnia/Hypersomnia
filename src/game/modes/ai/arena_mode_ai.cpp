#include "game/components/movement_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/gun_component.h"
#include "game/components/sentience_component.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/cosmos/entity_handle.h"
#include "augs/math/math.h"
#include "augs/misc/randomization.h"
#include "game/modes/ai/arena_mode_ai.h"
#include "game/enums/filters.h"
#include "game/cosmos/for_each_entity.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "augs/misc/scope_guard.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/debug_drawing_settings.h"
#include "game/messages/gunshot_message.h"
#include "game/messages/game_notification.h"
#include "game/detail/inventory/weapon_reloading.hpp"
#include "game/detail/pathfinding.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/inventory/perform_wielding.hpp"

#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/tasks/update_random_movement_target.hpp"
#include "game/modes/ai/tasks/find_closest_enemy.hpp"
#include "game/modes/ai/tasks/update_alertness.hpp"
#include "game/modes/ai/tasks/update_target_tracking.hpp"
#include "game/modes/ai/tasks/handle_active_chase.hpp"
#include "game/modes/ai/tasks/handle_random_movement.hpp"
#include "game/modes/ai/tasks/handle_aiming_and_trigger.hpp"
#include "game/modes/ai/tasks/interpolate_crosshair.hpp"
#include "game/modes/ai/tasks/handle_purchases.hpp"
#include "game/modes/ai/tasks/listen_for_footsteps.hpp"
#include "game/modes/ai/tasks/ai_pathfinding.hpp"
#include "game/modes/ai/tasks/navigate_pathfinding.hpp"
#include "game/modes/ai/tasks/ai_behavior_tree.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"

arena_ai_result update_arena_mode_ai(
	cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	arena_mode_ai_team_state& team_state,
	const entity_id controlled_character_id,
	const mode_player_id& bot_player_id,
	const faction_type bot_faction,
	const money_type money,
	const bool is_ffa,
	xorshift_state& stable_round_rng,
	const difficulty_type difficulty,
	const cosmos_navmesh& navmesh,
	const bool bomb_planted,
	const entity_id bomb_entity
) {
	auto stable_rng = randomization(stable_round_rng);

	auto scope = augs::scope_guard([&]() {
		stable_round_rng = stable_rng.generator;
	});

	const auto character_handle = cosm[controlled_character_id];

	if (!character_handle.alive()) {
		return arena_ai_result{};
	}

	auto& movement = character_handle.get<components::movement>();
	const auto character_pos = character_handle.get_logic_transform().pos;
	const auto dt_secs = step.get_delta().in_seconds();
	const auto& physics = cosm.get_solvable_inferred().physics;

	const auto ctx = ai_character_context{
		ai_state,
		character_pos,
		physics,
		cosm,
		character_handle
	};

	auto set_movement_target = [&](const vec2 target_pos, const bool sprint, const bool dash) {
		auto offset = target_pos - character_pos;
		ai_state.target_crosshair_offset = offset;
		movement.flags.sprinting = sprint;
		movement.flags.dashing = dash;
		return offset.normalize();
	};

	const bool is_metropolis = (bot_faction == faction_type::METROPOLIS);
	const bool is_resistance = (bot_faction == faction_type::RESISTANCE);

	/*
		Combat timeout management.
	*/
	if (ai_state.combat_timeout > 0.0f) {
		ai_state.combat_timeout -= dt_secs;

		if (ai_state.combat_timeout <= 0.0f) {
			ai_state.end_combat();
			/*
				After combat expires, unassign waypoint but keep patrol letter.
			*/
			::unassign_bot_from_waypoints(team_state, bot_player_id);
			ai_state.current_waypoint = entity_id::dead();
			ai_state.going_to_first_waypoint = true;
		}
	}

	/*
		Camp timer management.
	*/
	if (ai_state.camp_timer > 0.0f) {
		ai_state.camp_timer -= dt_secs;
	}

	/*
		Weapon management based on behavior state.
	*/
	const bool should_holster = ::should_holster_weapons(ai_state);
	const auto current_wielding = wielding_setup::from_current(character_handle);
	const bool has_bare_hands = current_wielding.is_bare_hands(cosm);

	if (should_holster && !has_bare_hands) {
		::perform_wielding(
			step,
			character_handle,
			wielding_setup::bare_hands()
		);
	}
	else if (!should_holster && has_bare_hands) {
		const auto best_weapon = ::find_best_weapon(character_handle);

		if (best_weapon.is_set()) {
			auto requested_wield = wielding_setup::bare_hands();
			requested_wield.hand_selections[0] = best_weapon;

			::perform_wielding(
				step,
				character_handle,
				requested_wield
			);
		}
	}

	/*
		Movement mode based on state.
	*/
	movement.flags.walking = ::should_walk_silently(ai_state);

	/*
		Listen for footsteps before checking for visible enemies.
		If hearing event point is in LoS, it can initiate combat.
	*/
	::listen_for_footsteps(ctx, step, is_ffa);

	/*
		If we heard something and can see that point, initiate combat.
	*/
	if (ai_state.last_seen_target.is_set() && ai_state.current_state != bot_state_type::COMBAT) {
		const auto heard_pos = ai_state.last_target_position;

		if (::is_in_line_of_sight(character_pos, heard_pos, physics, cosm, character_handle)) {
			ai_state.combat_timeout = stable_rng.randval(5.0f, 10.0f);
			ai_state.start_combat(ai_state.last_seen_target, heard_pos);
		}
	}

	::update_random_movement_target(ctx, dt_secs, navmesh);

	/*
		Check for closest visible enemy.
		Use extended FOV if camping.
	*/
	const bool camping = ::is_camping(ai_state);
	const auto closest_enemy = ::find_closest_enemy(ctx, is_ffa);
	const bool sees_target = closest_enemy.is_set();
	const bool should_react = ::update_alertness(ai_state, sees_target, dt_secs, difficulty);
	const bool has_target = sees_target && should_react;

	/*
		COMBAT initiation: Only SIGHTING initiates combat.
	*/
	if (sees_target && should_react) {
		const auto enemy_handle = cosm[closest_enemy];
		const auto enemy_pos = enemy_handle.get_logic_transform().pos;

		if (!ai_state.is_in_combat()) {
			/*
				Initiate combat with random duration 5-10 seconds.
			*/
			ai_state.combat_timeout = stable_rng.randval(5.0f, 10.0f);
			ai_state.start_combat(closest_enemy, enemy_pos);
			::unassign_bot_from_waypoints(team_state, bot_player_id);
		}
		else {
			/*
				Update last_known and last_seen positions since we can see them.
				Only override if closer.
			*/
			const auto current_dist = (enemy_pos - character_pos).length();
			const auto known_dist = (ai_state.last_known_target_pos - character_pos).length();

			if (current_dist < known_dist) {
				ai_state.last_seen_target_pos = enemy_pos;
				ai_state.last_known_target_pos = enemy_pos;
				ai_state.combat_target = closest_enemy;
				ai_state.has_dashed_for_last_seen = false;
			}
		}

		::update_target_tracking(ctx, closest_enemy);
		ai_state.has_dashed_for_last_seen_target = false;
	}

	const bool reloading = ::is_currently_reloading(character_handle);

	if (ai_state.chase_timeout > 0.0f) {
		ai_state.chase_timeout -= dt_secs;
	}

	const bool pause_chase = ai_state.chase_timeout > 0.0f || reloading;

	if (pause_chase) {
		ai_state.chase_remaining_time -= dt_secs;
	}

	/*
		Dash once when crossing last_seen_target_pos in combat.
	*/
	const bool should_dash = ::should_dash_for_combat(ai_state, character_pos);

	/*
		Set movement flags based on current state.
	*/
	movement.flags.sprinting = ::should_sprint(ai_state);
	movement.flags.dashing = should_dash;

	/*
		===========================================================================
		BEHAVIOR TREE: Evaluate entire tree every step.
		===========================================================================
	*/

	/*
		1) COMBAT state overrides all other pathfindings/patrols/objectives.
	*/
	if (ai_state.is_in_combat()) {
		/*
			Pathfind to last_known_target_pos.
		*/
		const auto target_pos = ai_state.last_known_target_pos;

		if (::start_pathfinding_to(ai_state, character_pos, target_pos, navmesh, nullptr)) {
			if (ai_state.is_pathfinding_active()) {
				vec2 crosshair_offset;
				const auto movement_dir = ::get_pathfinding_movement_direction(
					*ai_state.pathfinding,
					character_pos,
					navmesh,
					crosshair_offset,
					dt_secs
				);

				if (movement_dir.has_value()) {
					ai_state.target_crosshair_offset = crosshair_offset;
					::debug_draw_pathfinding(ai_state.pathfinding, character_pos, navmesh);
					movement.flags.set_from_closest_direction(*movement_dir);
				}
			}
		}

		::handle_aiming_and_trigger(ctx, has_target, closest_enemy);
		::interpolate_crosshair(ctx, has_target, dt_secs, difficulty);

		arena_ai_result result;
		result.item_purchase = ::handle_purchases(ctx, money, dt_secs, stable_rng);
		return result;
	}

	/*
		2) If not in combat, continue with map-objective pathfinding logic.
	*/

	/*
		Initialize patrol if not done yet.
	*/
	if (ai_state.current_state == bot_state_type::IDLE) {
		/*
			Metropolis: 20% chance to choose push waypoint if available.
		*/
		if (is_metropolis) {
			const bool choose_push = stable_rng.randval(0, 99) < 20;

			if (choose_push) {
				const auto push_wp = ::find_random_unassigned_push_waypoint(team_state, stable_rng);

				if (push_wp.is_set()) {
					ai_state.current_waypoint = push_wp;
					ai_state.current_state = bot_state_type::PUSHING;
					ai_state.going_to_first_waypoint = true;
					::assign_waypoint(team_state, push_wp, bot_player_id);
				}
			}

			if (ai_state.current_state == bot_state_type::IDLE) {
				/*
					Choose bombsite with least assigned soldiers.
				*/
				ai_state.patrol_letter = ::find_least_assigned_bombsite(cosm, team_state);
				ai_state.current_state = bot_state_type::PATROLLING;
				ai_state.going_to_first_waypoint = true;
			}
		}
		/*
			Resistance: Choose push waypoint first.
		*/
		else if (is_resistance) {
			const auto push_wp = ::find_random_unassigned_push_waypoint(team_state, stable_rng);

			if (push_wp.is_set()) {
				ai_state.current_waypoint = push_wp;
				ai_state.current_state = bot_state_type::PUSHING;
				ai_state.going_to_first_waypoint = true;
				::assign_waypoint(team_state, push_wp, bot_player_id);
			}
			else {
				/*
					Fallback to patrol of team's chosen_bombsite.
				*/
				ai_state.patrol_letter = team_state.chosen_bombsite;
				ai_state.current_state = bot_state_type::PATROLLING;
				ai_state.going_to_first_waypoint = true;
			}
		}
		else {
			/*
				FFA or other: Just patrol.
			*/
			ai_state.patrol_letter = ::find_least_assigned_bombsite(cosm, team_state);
			ai_state.current_state = bot_state_type::PATROLLING;
			ai_state.going_to_first_waypoint = true;
		}
	}

	/*
		3.2) If bomb planted (Metropolis-specific: defuse mission).
	*/
	if (bomb_planted && is_metropolis) {
		/*
			Switch to patrol the planted bombsite letter.
		*/
		ai_state.patrol_letter = team_state.chosen_bombsite;

		/*
			Check if we should be the defuse bot.
		*/
		const bool defuse_bot_alive = [&]() {
			if (!team_state.bot_with_defuse_mission.is_set()) {
				return false;
			}

			return true;
		}();

		if (!defuse_bot_alive) {
			/*
				Assign closest bot to defuse mission.
			*/
			team_state.bot_with_defuse_mission = bot_player_id;
		}

		if (team_state.bot_with_defuse_mission == bot_player_id) {
			ai_state.current_state = bot_state_type::DEFUSING;

			/*
				Pathfind to bomb instead of waypoint.
			*/
			const auto bomb_handle = cosm[bomb_entity];

			if (bomb_handle.alive()) {
				const auto bomb_pos = bomb_handle.get_logic_transform().pos;

				if (::start_pathfinding_to(ai_state, character_pos, bomb_pos, navmesh, nullptr)) {
					if (ai_state.is_pathfinding_active()) {
						vec2 crosshair_offset;
						const auto movement_dir = ::get_pathfinding_movement_direction(
							*ai_state.pathfinding,
							character_pos,
							navmesh,
							crosshair_offset,
							dt_secs
						);

						if (movement_dir.has_value()) {
							ai_state.target_crosshair_offset = crosshair_offset;
							::debug_draw_pathfinding(ai_state.pathfinding, character_pos, navmesh);
							movement.flags.set_from_closest_direction(*movement_dir);
						}
					}
				}

				/*
					If close to bomb, enter defusing state.
				*/
				if (::has_reached_waypoint(character_pos, bomb_pos, 100.0f)) {
					ai_state.is_defusing = true;
					ai_state.target_crosshair_offset = bomb_pos - character_pos;
					movement.flags.set_from_closest_direction(vec2::zero);

					if (auto* sentience = character_handle.find<components::sentience>()) {
						sentience->is_requesting_interaction = true;
					}
				}
			}

			::handle_aiming_and_trigger(ctx, has_target, closest_enemy);
			::interpolate_crosshair(ctx, has_target, dt_secs, difficulty);

			arena_ai_result result;
			result.item_purchase = ::handle_purchases(ctx, money, dt_secs, stable_rng);
			return result;
		}
	}

	/*
		PUSHING state: pathfind to push waypoint.
	*/
	if (ai_state.current_state == bot_state_type::PUSHING) {
		const auto wp_handle = cosm[ai_state.current_waypoint];

		if (wp_handle.alive()) {
			const auto wp_pos = wp_handle.get_logic_transform().pos;

			if (::has_reached_waypoint(character_pos, wp_pos)) {
				/*
					Finished push, check if camp.
				*/
				if (::is_camp_waypoint(cosm, ai_state.current_waypoint)) {
					ai_state.camp_timer = stable_rng.randval(5.0f, 15.0f);
					ai_state.camp_center = wp_pos;
					ai_state.camp_twitch_target = wp_pos;
				}

				/*
					Switch to patrolling.
				*/
				::unassign_bot_from_waypoints(team_state, bot_player_id);
				ai_state.current_waypoint = entity_id::dead();
				ai_state.current_state = bot_state_type::PATROLLING;
				ai_state.going_to_first_waypoint = true;
				ai_state.patrol_letter = is_resistance ? team_state.chosen_bombsite : ::find_least_assigned_bombsite(cosm, team_state);
			}
			else {
				/*
					Continue pathfinding to waypoint.
				*/
				if (::start_pathfinding_to(ai_state, character_pos, wp_pos, navmesh, nullptr)) {
					if (ai_state.is_pathfinding_active()) {
						vec2 crosshair_offset;
						const auto movement_dir = ::get_pathfinding_movement_direction(
							*ai_state.pathfinding,
							character_pos,
							navmesh,
							crosshair_offset,
							dt_secs
						);

						if (movement_dir.has_value()) {
							ai_state.target_crosshair_offset = crosshair_offset;
							::debug_draw_pathfinding(ai_state.pathfinding, character_pos, navmesh);
							movement.flags.set_from_closest_direction(*movement_dir);
						}
					}
				}
			}
		}
		else {
			/*
				Waypoint gone, switch to patrolling.
			*/
			ai_state.current_state = bot_state_type::PATROLLING;
			ai_state.going_to_first_waypoint = true;
		}

		::handle_aiming_and_trigger(ctx, has_target, closest_enemy);
		::interpolate_crosshair(ctx, has_target, dt_secs, difficulty);

		arena_ai_result result;
		result.item_purchase = ::handle_purchases(ctx, money, dt_secs, stable_rng);
		return result;
	}

	/*
		PATROLLING state.
	*/
	if (ai_state.current_state == bot_state_type::PATROLLING) {
		/*
			If camping, do camp twitching.
		*/
		if (camping) {
			const auto twitch_dir = ::update_camp_twitch(ai_state, character_pos, stable_rng);
			movement.flags.set_from_closest_direction(twitch_dir);
			movement.flags.walking = true;

			::handle_aiming_and_trigger(ctx, has_target, closest_enemy);
			::interpolate_crosshair(ctx, has_target, dt_secs, difficulty);

			arena_ai_result result;
			result.item_purchase = ::handle_purchases(ctx, money, dt_secs, stable_rng);
			return result;
		}

		/*
			Need to find a waypoint if we don't have one.
		*/
		if (!ai_state.current_waypoint.is_set()) {
			auto new_wp = ::find_random_unassigned_patrol_waypoint(
				cosm,
				team_state,
				ai_state.patrol_letter,
				bot_player_id,
				stable_rng
			);

			if (new_wp.is_set()) {
				ai_state.current_waypoint = new_wp;
				::assign_waypoint(team_state, new_wp, bot_player_id);

				/*
					85% chance to walk silently when not going to first waypoint.
				*/
				if (!ai_state.going_to_first_waypoint) {
					ai_state.walk_silently_to_next_waypoint = stable_rng.randval(0, 99) < 85;
				}
			}
		}

		const auto wp_handle = cosm[ai_state.current_waypoint];

		if (wp_handle.alive()) {
			const auto wp_pos = wp_handle.get_logic_transform().pos;

			if (::has_reached_waypoint(character_pos, wp_pos)) {
				ai_state.going_to_first_waypoint = false;

				/*
					Check if camp waypoint.
				*/
				if (::is_camp_waypoint(cosm, ai_state.current_waypoint)) {
					ai_state.camp_timer = stable_rng.randval(5.0f, 15.0f);
					ai_state.camp_duration = ai_state.camp_timer;
					ai_state.camp_center = wp_pos;
					ai_state.camp_twitch_target = wp_pos;
				}
				else {
					/*
						Pick next waypoint immediately.
					*/
					::unassign_bot_from_waypoints(team_state, bot_player_id);
					ai_state.current_waypoint = entity_id::dead();
				}
			}
			else {
				/*
					Pathfind to current waypoint.
				*/
				if (::start_pathfinding_to(ai_state, character_pos, wp_pos, navmesh, nullptr)) {
					if (ai_state.is_pathfinding_active()) {
						vec2 crosshair_offset;
						const auto movement_dir = ::get_pathfinding_movement_direction(
							*ai_state.pathfinding,
							character_pos,
							navmesh,
							crosshair_offset,
							dt_secs
						);

						if (movement_dir.has_value()) {
							ai_state.target_crosshair_offset = crosshair_offset;
							::debug_draw_pathfinding(ai_state.pathfinding, character_pos, navmesh);
							movement.flags.set_from_closest_direction(*movement_dir);
						}
					}
				}
			}
		}
		else {
			/*
				Waypoint doesn't exist, find another.
			*/
			ai_state.current_waypoint = entity_id::dead();
		}

		::handle_aiming_and_trigger(ctx, has_target, closest_enemy);
		::interpolate_crosshair(ctx, has_target, dt_secs, difficulty);

		arena_ai_result result;
		result.item_purchase = ::handle_purchases(ctx, money, dt_secs, stable_rng);
		return result;
	}

	/*
		Fallback: process pathfinding navigation.
	*/
	const auto nav_result = ::navigate_pathfinding(
		ai_state.pathfinding,
		character_pos,
		navmesh,
		character_handle,
		ai_state.target_crosshair_offset,
		dt_secs
	);

	if (nav_result.is_navigating) {
		::handle_aiming_and_trigger(ctx, has_target, closest_enemy);
		::interpolate_crosshair(ctx, has_target, dt_secs, difficulty);

		arena_ai_result result;
		result.item_purchase = ::handle_purchases(ctx, money, dt_secs, stable_rng);
		return result;
	}

	/*
		Handle chase logic with pathfinding (legacy fallback).
	*/
	const auto actual_movement_direction = [&]() {
		if (!pause_chase && ai_state.last_seen_target.is_set()) {
			/*
				Start pathfinding to last known target position.
			*/
			const auto target_pos = ai_state.last_target_position;

			if (::start_pathfinding_to(ai_state, character_pos, target_pos, navmesh, nullptr)) {
				if (ai_state.is_pathfinding_active()) {
					vec2 crosshair_offset;
					const auto movement_dir = ::get_pathfinding_movement_direction(
						*ai_state.pathfinding,
						character_pos,
						navmesh,
						crosshair_offset,
						dt_secs
					);

					if (movement_dir.has_value()) {
						ai_state.target_crosshair_offset = crosshair_offset;
						::debug_draw_pathfinding(ai_state.pathfinding, character_pos, navmesh);
						return *movement_dir;
					}
				}
			}

			/*
				Fallback to direct chase if pathfinding fails.
			*/
			return ::handle_active_chase(ctx, sees_target, dt_secs, set_movement_target);
		}
		else {
			return ::handle_random_movement(ctx, pause_chase, set_movement_target);
		}
	}();

	movement.flags.set_from_closest_direction(actual_movement_direction);

	::handle_aiming_and_trigger(ctx, has_target, closest_enemy);
	::interpolate_crosshair(ctx, has_target, dt_secs, difficulty);

	arena_ai_result result;
	result.item_purchase = ::handle_purchases(ctx, money, dt_secs, stable_rng);

	return result;
}

void post_solve_arena_mode_ai(
	cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	const entity_id controlled_character_id,
	const bool is_ffa
) {
	const auto character_handle = cosm[controlled_character_id];

	if (!character_handle.alive()) {
		return;
	}

	/*
		Check for teleportation messages - clear pathfinding if this bot was teleported.
	*/
	const auto& game_notifications = step.get_queue<messages::game_notification>();

	for (const auto& notification : game_notifications) {
		if (const auto* tp = std::get_if<messages::teleportation>(&notification.payload)) {
			if (tp->teleported == controlled_character_id) {
				/*
					Bot was teleported - clear pathfinding state.
					New pathfinding will be initiated on next update.
				*/
				ai_state.clear_pathfinding();
				break;
			}
		}
	}

	const auto& gunshots = step.get_queue<messages::gunshot_message>();
	const auto bot_faction = character_handle.get_official_faction();
	const auto character_pos = character_handle.get_logic_transform().pos;
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto filter = predefined_queries::line_of_sight();

	auto current_target_distance = [&]() {
		if (ai_state.last_seen_target.is_set()) {
			return (ai_state.last_target_position - character_pos).length();
		}

		return std::numeric_limits<float>::max();
	}();

	auto is_enemy_faction = [&](const faction_type shooter_faction) {
		if (is_ffa) {
			return bot_faction != shooter_faction;
		}

		return bot_faction != shooter_faction && shooter_faction != faction_type::SPECTATOR;
	};

	auto update_target_from_gunshot = [&](const entity_id shooter_id, const vec2 muzzle_pos, const float dist) {
		ai_state.last_seen_target = shooter_id;
		ai_state.chase_remaining_time = 5.0f;
		ai_state.last_target_position = muzzle_pos;
		ai_state.has_dashed_for_last_seen_target = false;
		current_target_distance = dist;
	};

	for (const auto& shot : gunshots) {
		if (!shot.capability.is_set()) {
			continue;
		}

		const auto shooter = cosm[shot.capability];

		if (!shooter.alive() || shooter == character_handle) {
			continue;
		}

		if (!is_enemy_faction(shooter.get_official_faction())) {
			continue;
		}

		const auto muzzle_pos = shot.muzzle_transform.pos;
		const auto raycast = physics.ray_cast_px(
			cosm.get_si(),
			character_pos,
			muzzle_pos,
			filter,
			character_handle
		);

		if (!raycast.hit) {
			const auto dist = (muzzle_pos - character_pos).length();

			if (!ai_state.last_seen_target.is_set() || dist < current_target_distance) {
				update_target_from_gunshot(shot.capability, muzzle_pos, dist);
			}
		}
	}

	if (auto* movement = character_handle.find<components::movement>()) {
		movement->flags.sprinting = false;
		movement->flags.dashing = false;
	}
}
