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
#include "game/messages/health_event.h"
#include "game/detail/inventory/weapon_reloading.hpp"
#include "game/detail/pathfinding.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/inventory/perform_wielding.hpp"

#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/tasks/find_closest_enemy.hpp"
#include "game/modes/ai/tasks/update_alertness.hpp"
#include "game/modes/ai/tasks/handle_aiming_and_trigger.hpp"
#include "game/modes/ai/tasks/interpolate_crosshair.hpp"
#include "game/modes/ai/tasks/handle_purchases.hpp"
#include "game/modes/ai/tasks/listen_for_footsteps.hpp"
#include "game/modes/ai/tasks/ai_pathfinding.hpp"
#include "game/modes/ai/tasks/ai_behavior_tree.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"
#include "game/modes/ai/tasks/navigate_pathfinding.hpp"

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

	const bool is_metropolis = (bot_faction == faction_type::METROPOLIS);
	const bool is_resistance = (bot_faction == faction_type::RESISTANCE);

	AI_LOG("=== update_arena_mode_ai ===");
	AI_LOG_NVPS(controlled_character_id, ai_state.current_state, bot_faction);

	/*
		===========================================================================
		LAMBDAS: Reusable behavior blocks.
		===========================================================================
	*/

	/*
		Stateless calculation of movement direction.
		Returns the direction from navigate_pathfinding if pathfinding is active,
		or from camp twitching if camping, or nullopt if no movement needed.
	*/
	std::optional<vec2> current_movement_direction;
	bool is_actively_pathfinding = false;

	/* Pathfind to target position. 
	   Uses navigate_pathfinding for proper path following with advancement and deviation checks.
	   Sets current_movement_direction and is_actively_pathfinding as side effects. */
	auto pathfind_to = [&](const vec2 target_pos, const bool exact = false) -> bool {
		/* Start pathfinding if not already active or if target changed. */
		::start_pathfinding_to(ai_state, character_pos, target_pos, navmesh, nullptr);

		if (!ai_state.is_pathfinding_active()) {
			return false;
		}

		/* Set exact flag if requested (for waypoints). */
		ai_state.pathfinding->exact_destination = exact;

		/* Use navigate_pathfinding for proper path following. */
		const auto nav_result = ::navigate_pathfinding(
			ai_state.pathfinding,
			character_pos,
			navmesh,
			character_handle,
			dt_secs
		);

		if (nav_result.path_completed) {
			return false;
		}

		if (nav_result.is_navigating) {
			ai_state.target_crosshair_offset = nav_result.crosshair_offset;
			current_movement_direction = nav_result.movement_direction;
			is_actively_pathfinding = true;
			return true;
		}

		return false;
	};

	/* Finalize frame: apply movement, handle aiming, crosshair, and purchases. */
	auto finalize_frame = [&](const bool has_target, const entity_id closest_enemy) -> arena_ai_result {
		/* Apply movement direction and flags. */
		movement.flags.walking = ::should_walk_silently(ai_state);
		movement.flags.sprinting = ::should_sprint(ai_state);
		movement.flags.dashing = ::should_dash_for_combat(ai_state, character_pos);

		if (current_movement_direction.has_value()) {
			movement.flags.set_from_closest_direction(*current_movement_direction);
		}
		else {
			movement.flags.set_from_closest_direction(vec2::zero);
		}

		AI_LOG_NVPS(movement.flags.walking, movement.flags.sprinting, movement.flags.dashing);

		::handle_aiming_and_trigger(ctx, has_target, closest_enemy);
		::interpolate_crosshair(ctx, has_target, dt_secs, difficulty, is_actively_pathfinding);

		arena_ai_result result;
		result.item_purchase = ::handle_purchases(ctx, money, dt_secs, stable_rng);
		return result;
	};

	/* Manage weapon holstering/drawing based on state. */
	auto manage_weapons = [&]() {
		const bool should_holster = ::should_holster_weapons(ai_state);
		const auto current_wielding = wielding_setup::from_current(character_handle);
		const bool has_bare_hands = current_wielding.is_bare_hands(cosm);

		if (should_holster && !has_bare_hands) {
			AI_LOG("Holstering weapons");
			::perform_wielding(step, character_handle, wielding_setup::bare_hands());
		}
		else if (!should_holster && has_bare_hands) {
			const auto best_weapon = ::find_best_weapon(character_handle);

			if (best_weapon.is_set()) {
				AI_LOG_NVPS("Drawing weapon", best_weapon);
				auto requested_wield = wielding_setup::bare_hands();
				requested_wield.hand_selections[0] = best_weapon;
				::perform_wielding(step, character_handle, requested_wield);
			}
		}
	};

	/* Handle combat timeout expiration. */
	auto update_combat_timeout = [&]() {
		if (ai_state.combat_timeout > 0.0f) {
			ai_state.combat_timeout -= dt_secs;

			if (ai_state.combat_timeout <= 0.0f) {
				AI_LOG("Combat timeout expired - resetting to IDLE");
				ai_state.end_combat();
				::unassign_bot_from_waypoints(team_state, bot_player_id);
				ai_state.current_waypoint = entity_id::dead();
				ai_state.going_to_first_waypoint = true;
			}
		}
	};

	/* Handle camp timer. */
	auto update_camp_timer = [&]() {
		if (ai_state.camp_timer > 0.0f) {
			ai_state.camp_timer -= dt_secs;
		}
	};

	/* Check for hearing events that could initiate combat. */
	auto check_hearing_initiates_combat = [&]() {
		::listen_for_footsteps(ctx, step, is_ffa);

		if (ai_state.last_seen_target.is_set() && ai_state.current_state != bot_state_type::COMBAT) {
			const auto heard_pos = ai_state.last_target_position;

			if (::is_in_line_of_sight(character_pos, heard_pos, physics, cosm, character_handle)) {
				AI_LOG("Heard sound in LoS - initiating combat");
				ai_state.combat_timeout = stable_rng.randval(5.0f, 10.0f);
				ai_state.start_combat(ai_state.last_seen_target, heard_pos);
			}
		}
	};

	/* Check for visible enemies and initiate/update combat. */
	auto check_sighting_initiates_combat = [&](const entity_id closest_enemy, const bool sees_target, const bool should_react) {
		if (sees_target && should_react) {
			const auto enemy_handle = cosm[closest_enemy];
			const auto enemy_pos = enemy_handle.get_logic_transform().pos;

			if (!ai_state.is_in_combat()) {
				AI_LOG("SIGHTING - Initiating COMBAT");
				AI_LOG_NVPS(closest_enemy, enemy_pos);
				ai_state.combat_timeout = stable_rng.randval(5.0f, 10.0f);
				ai_state.start_combat(closest_enemy, enemy_pos);
				::unassign_bot_from_waypoints(team_state, bot_player_id);
			}
			else {
				/* Update target only if closer. */
				const auto current_dist = (enemy_pos - character_pos).length();
				const auto known_dist = (ai_state.last_known_target_pos - character_pos).length();

				if (current_dist < known_dist) {
					AI_LOG("Updating combat target (closer)");
					AI_LOG_NVPS(closest_enemy, enemy_pos, current_dist, known_dist);
					ai_state.last_seen_target_pos = enemy_pos;
					ai_state.last_known_target_pos = enemy_pos;
					ai_state.combat_target = closest_enemy;
					ai_state.has_dashed_for_last_seen = false;
				}
			}
		}
	};

	/* Initialize patrol from IDLE state. */
	auto init_patrol_from_idle = [&]() {
		if (ai_state.current_state != bot_state_type::IDLE) {
			return;
		}

		AI_LOG("Initializing from IDLE state");

		if (is_metropolis) {
			/* Metropolis: 20% chance to choose push waypoint. */
			const bool choose_push = stable_rng.randval(0, 99) < 20;

			if (choose_push) {
				const auto push_wp = ::find_random_unassigned_push_waypoint(team_state, stable_rng);

				if (push_wp.is_set()) {
					AI_LOG("Metropolis -> PUSHING");
					AI_LOG_NVPS(push_wp);
					ai_state.current_waypoint = push_wp;
					ai_state.current_state = bot_state_type::PUSHING;
					ai_state.going_to_first_waypoint = true;
					::assign_waypoint(team_state, push_wp, bot_player_id);
					return;
				}
			}

			/* Default: patrol least-assigned bombsite. */
			ai_state.patrol_letter = ::find_least_assigned_bombsite(cosm, team_state);
			ai_state.current_state = bot_state_type::PATROLLING;
			ai_state.going_to_first_waypoint = true;
			AI_LOG("Metropolis -> PATROLLING");
			AI_LOG_NVPS(ai_state.patrol_letter);
		}
		else if (is_resistance) {
			/* Resistance: choose push waypoint first. */
			const auto push_wp = ::find_random_unassigned_push_waypoint(team_state, stable_rng);

			if (push_wp.is_set()) {
				AI_LOG("Resistance -> PUSHING");
				AI_LOG_NVPS(push_wp);
				ai_state.current_waypoint = push_wp;
				ai_state.current_state = bot_state_type::PUSHING;
				ai_state.going_to_first_waypoint = true;
				::assign_waypoint(team_state, push_wp, bot_player_id);
			}
			else {
				ai_state.patrol_letter = team_state.chosen_bombsite;
				ai_state.current_state = bot_state_type::PATROLLING;
				ai_state.going_to_first_waypoint = true;
				AI_LOG("Resistance -> PATROLLING (fallback)");
				AI_LOG_NVPS(ai_state.patrol_letter);
			}
		}
		else {
			/* FFA or other: patrol. */
			ai_state.patrol_letter = ::find_least_assigned_bombsite(cosm, team_state);
			ai_state.current_state = bot_state_type::PATROLLING;
			ai_state.going_to_first_waypoint = true;
			AI_LOG("FFA -> PATROLLING");
			AI_LOG_NVPS(ai_state.patrol_letter);
		}
	};

	/* Handle COMBAT state. */
	auto handle_combat = [&](const bool has_target, const entity_id closest_enemy) -> std::optional<arena_ai_result> {
		if (!ai_state.is_in_combat()) {
			return std::nullopt;
		}

		AI_LOG("In COMBAT state");
		AI_LOG_NVPS(ai_state.last_known_target_pos, ai_state.combat_timeout);

		pathfind_to(ai_state.last_known_target_pos);
		return finalize_frame(has_target, closest_enemy);
	};

	/* Handle Resistance bomb retrieval mission. */
	auto handle_bomb_retrieval = [&](const bool has_target, const entity_id closest_enemy) -> std::optional<arena_ai_result> {
		if (bomb_planted || !is_resistance || !bomb_entity.is_set()) {
			return std::nullopt;
		}

		const auto bomb_handle = cosm[bomb_entity];

		if (!bomb_handle.alive()) {
			return std::nullopt;
		}

		const auto bomb_owner = bomb_handle.get_owning_transfer_capability();
		const bool bomb_on_ground = !bomb_owner.alive();
		const bool bomb_held_by_enemy = bomb_owner.alive() && 
			bomb_owner.get_official_faction() == faction_type::METROPOLIS;

		if (!bomb_on_ground && !bomb_held_by_enemy) {
			/* Bomb is held by Resistance - clear mission. */
			if (team_state.bot_with_bomb_retrieval_mission.is_set()) {
				AI_LOG("Clearing bomb retrieval mission - bomb secured");
				team_state.bot_with_bomb_retrieval_mission = mode_player_id::dead();
			}

			return std::nullopt;
		}

		/* Assign retrieval mission if not assigned. */
		if (!team_state.bot_with_bomb_retrieval_mission.is_set()) {
			team_state.bot_with_bomb_retrieval_mission = bot_player_id;
			AI_LOG("Assigned to bomb retrieval mission");
		}

		if (team_state.bot_with_bomb_retrieval_mission != bot_player_id) {
			return std::nullopt;
		}

		AI_LOG("RETRIEVING_BOMB state");
		ai_state.current_state = bot_state_type::RETRIEVING_BOMB;

		const auto bomb_pos = bomb_handle.get_logic_transform().pos;
		AI_LOG_NVPS(bomb_pos);

		pathfind_to(bomb_pos);
		return finalize_frame(has_target, closest_enemy);
	};

	/* Handle Metropolis defuse mission when bomb is planted. */
	auto handle_defuse_mission = [&](const bool has_target, const entity_id closest_enemy) -> std::optional<arena_ai_result> {
		if (!bomb_planted || !is_metropolis) {
			return std::nullopt;
		}

		AI_LOG("Bomb planted - Metropolis defuse logic");
		ai_state.patrol_letter = team_state.chosen_bombsite;

		/* Assign defuse mission if not assigned. */
		if (!team_state.bot_with_defuse_mission.is_set()) {
			team_state.bot_with_defuse_mission = bot_player_id;
			AI_LOG("Assigned to defuse mission");
		}

		if (team_state.bot_with_defuse_mission != bot_player_id) {
			return std::nullopt;
		}

		AI_LOG("DEFUSING state");
		ai_state.current_state = bot_state_type::DEFUSING;

		const auto bomb_handle = cosm[bomb_entity];

		if (!bomb_handle.alive()) {
			return std::nullopt;
		}

		const auto bomb_pos = bomb_handle.get_logic_transform().pos;

		/* If close to bomb, enter defusing state. */
		if (::has_reached_waypoint(character_pos, bomb_pos, 100.0f)) {
			AI_LOG("Close to bomb - starting defuse");
			ai_state.is_defusing = true;
			ai_state.target_crosshair_offset = bomb_pos - character_pos;
			/* Stop moving - use nullopt to indicate no movement needed. */
			current_movement_direction = std::nullopt;

			/* Bare hands for defusing. */
			const auto current_wielding_defuse = wielding_setup::from_current(character_handle);

			if (!current_wielding_defuse.is_bare_hands(cosm)) {
				::perform_wielding(step, character_handle, wielding_setup::bare_hands());
			}

			if (auto* sentience = character_handle.find<components::sentience>()) {
				sentience->is_requesting_interaction = true;
			}

			return finalize_frame(has_target, closest_enemy);
		}

		AI_LOG_NVPS(bomb_pos);
		pathfind_to(bomb_pos);
		return finalize_frame(has_target, closest_enemy);
	};

	/* Handle PUSHING state. */
	auto handle_pushing = [&](const bool has_target, const entity_id closest_enemy) -> std::optional<arena_ai_result> {
		if (ai_state.current_state != bot_state_type::PUSHING) {
			return std::nullopt;
		}

		AI_LOG("PUSHING state");

		const auto wp_handle = cosm[ai_state.current_waypoint];

		if (!wp_handle.alive()) {
			AI_LOG("Push waypoint gone - switching to PATROLLING");
			ai_state.current_state = bot_state_type::PATROLLING;
			ai_state.going_to_first_waypoint = true;
			return std::nullopt;
		}

		const auto wp_transform = wp_handle.get_logic_transform();
		const auto wp_pos = wp_transform.pos;

		/* Use pathfinding with exact=true for waypoints. */
		const bool still_pathfinding = pathfind_to(wp_pos, true);

		/* Pathfinding completed means we reached the waypoint. 
		   When pathfind_to returns false, it can mean: path completed, or couldn't start.
		   We check is_pathfinding_active to ensure path actually completed vs failed to start. */
		if (!still_pathfinding && !ai_state.is_pathfinding_active()) {
			AI_LOG("Reached push waypoint - switching to PATROLLING");

			if (::is_camp_waypoint(cosm, ai_state.current_waypoint)) {
				const auto [min_secs, max_secs] = ::get_waypoint_camp_duration_range(cosm, ai_state.current_waypoint);
				ai_state.camp_timer = stable_rng.randval(min_secs, max_secs);
				ai_state.camp_duration = ai_state.camp_timer;
				ai_state.camp_center = wp_pos;
				ai_state.camp_twitch_target = wp_pos;
				ai_state.camp_look_direction = wp_transform.get_direction();
				AI_LOG("Camp waypoint - setting up camp");
			}

			::unassign_bot_from_waypoints(team_state, bot_player_id);
			ai_state.current_waypoint = entity_id::dead();
			ai_state.current_state = bot_state_type::PATROLLING;
			ai_state.going_to_first_waypoint = true;
			ai_state.patrol_letter = is_resistance ? team_state.chosen_bombsite : ::find_least_assigned_bombsite(cosm, team_state);
			return std::nullopt;
		}

		AI_LOG_NVPS(wp_pos);
		return finalize_frame(has_target, closest_enemy);
	};

	/* Handle PATROLLING state. */
	auto handle_patrolling = [&](const bool has_target, const entity_id closest_enemy) -> std::optional<arena_ai_result> {
		if (ai_state.current_state != bot_state_type::PATROLLING) {
			return std::nullopt;
		}

		AI_LOG("PATROLLING state");
		AI_LOG_NVPS(ai_state.patrol_letter, ai_state.current_waypoint, ai_state.camp_timer);

		/* If camping, do camp twitching. */
		const bool camping = ::is_camping(ai_state);

		if (camping) {
			AI_LOG("Camping - twitching");
			const auto twitch_dir = ::update_camp_twitch(ai_state, character_pos, stable_rng);
			current_movement_direction = twitch_dir;

			/* Look in the direction of the waypoint transform while camping. */
			ai_state.target_crosshair_offset = ai_state.camp_look_direction * 200.0f;

			return finalize_frame(has_target, closest_enemy);
		}

		/* Check if camp timer just expired - need to pick next waypoint. */
		if (ai_state.current_waypoint.is_set() && ai_state.camp_duration > 0.0f && ai_state.camp_timer <= 0.0f) {
			AI_LOG("Camp duration expired - picking next waypoint");
			::unassign_bot_from_waypoints(team_state, bot_player_id);
			ai_state.current_waypoint = entity_id::dead();
			ai_state.camp_duration = 0.0f;
		}

		/* Find a waypoint if we don't have one. */
		if (!ai_state.current_waypoint.is_set()) {
			AI_LOG("No waypoint - finding one");
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
				AI_LOG_NVPS(new_wp);

				/* 85% chance to walk silently when not going to first waypoint. */
				if (!ai_state.going_to_first_waypoint) {
					ai_state.walk_silently_to_next_waypoint = stable_rng.randval(0, 99) < 85;
					AI_LOG_NVPS(ai_state.walk_silently_to_next_waypoint);
				}
			}
		}

		const auto wp_handle = cosm[ai_state.current_waypoint];

		if (!wp_handle.alive()) {
			AI_LOG("Waypoint doesn't exist - clearing");
			ai_state.current_waypoint = entity_id::dead();
			return finalize_frame(has_target, closest_enemy);
		}

		const auto wp_transform = wp_handle.get_logic_transform();
		const auto wp_pos = wp_transform.pos;

		/* Use pathfinding with exact=true for waypoints so completion is detected by reaching exact point. */
		const bool still_pathfinding = pathfind_to(wp_pos, true);

		/* Pathfinding completed means we reached the waypoint.
		   When pathfind_to returns false, it can mean: path completed, or couldn't start.
		   We check is_pathfinding_active to ensure path actually completed vs failed to start. */
		if (!still_pathfinding && !ai_state.is_pathfinding_active()) {
			AI_LOG("Reached waypoint (pathfinding completed)");
			ai_state.going_to_first_waypoint = false;

			if (::is_camp_waypoint(cosm, ai_state.current_waypoint)) {
				AI_LOG("Camp waypoint - setting up camp");
				const auto [min_secs, max_secs] = ::get_waypoint_camp_duration_range(cosm, ai_state.current_waypoint);
				ai_state.camp_timer = stable_rng.randval(min_secs, max_secs);
				ai_state.camp_duration = ai_state.camp_timer;
				ai_state.camp_center = wp_pos;
				ai_state.camp_twitch_target = wp_pos;
				/* Store the waypoint's facing direction for looking while camping. */
				ai_state.camp_look_direction = wp_transform.get_direction();
				AI_LOG_NVPS(ai_state.camp_timer, ai_state.camp_look_direction);
			}
			else {
				AI_LOG("Non-camp waypoint - picking next");
				::unassign_bot_from_waypoints(team_state, bot_player_id);
				ai_state.current_waypoint = entity_id::dead();
			}
		}

		return finalize_frame(has_target, closest_enemy);
	};

	/*
		===========================================================================
		BEHAVIOR TREE: Evaluate entire tree every step.
		===========================================================================
	*/

	/* Update timers. */
	update_combat_timeout();
	update_camp_timer();

	/* Manage weapons based on state. */
	manage_weapons();

	/* Check for hearing-initiated combat. */
	check_hearing_initiates_combat();

	/* Check for visible enemies. */
	const bool camping = ::is_camping(ai_state);
	const auto closest_enemy = ::find_closest_enemy(ctx, is_ffa);
	const bool sees_target = closest_enemy.is_set();
	const bool should_react = ::update_alertness(ai_state, sees_target, dt_secs, difficulty);
	const bool has_target = sees_target && should_react;

	AI_LOG_NVPS(sees_target, should_react, has_target, camping);

	/* Check if sighting initiates combat. */
	check_sighting_initiates_combat(closest_enemy, sees_target, should_react);

	/* Priority 1: COMBAT overrides everything. */
	if (auto result = handle_combat(has_target, closest_enemy)) {
		return *result;
	}

	/* Initialize patrol from IDLE if needed. */
	init_patrol_from_idle();

	/* Priority 2: Bomb retrieval (Resistance, bomb not planted). */
	if (auto result = handle_bomb_retrieval(has_target, closest_enemy)) {
		return *result;
	}

	/* Priority 3: Defuse mission (Metropolis, bomb planted). */
	if (auto result = handle_defuse_mission(has_target, closest_enemy)) {
		return *result;
	}

	/* Priority 4: PUSHING state. */
	if (auto result = handle_pushing(has_target, closest_enemy)) {
		return *result;
	}

	/* Priority 5: PATROLLING state. */
	if (auto result = handle_patrolling(has_target, closest_enemy)) {
		return *result;
	}

	/* Fallback: finalize frame with no specific behavior. */
	AI_LOG("Fallback - no specific state matched");
	return finalize_frame(has_target, closest_enemy);
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

	/*
		Damage-based combat initiation.
		Receiving damage always initiates COMBAT regardless of LoS.
	*/
	const auto& health_events = step.get_queue<messages::health_event>();

	for (const auto& event : health_events) {
		if (event.subject != controlled_character_id) {
			continue;
		}

		if (!event.origin.sender.capability_of_sender.is_set()) {
			continue;
		}

		const auto attacker_id = event.origin.sender.capability_of_sender;
		const auto attacker = cosm[attacker_id];

		if (!attacker.alive()) {
			continue;
		}

		if (attacker == character_handle) {
			continue;
		}

		if (!is_enemy_faction(attacker.get_official_faction())) {
			continue;
		}

		const auto attacker_pos = attacker.get_logic_transform().pos;
		const auto dist = (attacker_pos - character_pos).length();

		/*
			Damage always initiates combat (no LoS check needed).
		*/
		if (!ai_state.is_in_combat()) {
			ai_state.start_combat(attacker_id, attacker_pos);
			ai_state.combat_timeout = 7.5f;
		}
		else if (dist < current_target_distance) {
			/*
				Switch to closer attacker.
			*/
			ai_state.last_seen_target_pos = attacker_pos;
			ai_state.last_known_target_pos = attacker_pos;
			ai_state.combat_target = attacker_id;
			ai_state.has_dashed_for_last_seen = false;
			current_target_distance = dist;
		}
	}

	if (auto* movement = character_handle.find<components::movement>()) {
		movement->flags.sprinting = false;
		movement->flags.dashing = false;
	}
}
