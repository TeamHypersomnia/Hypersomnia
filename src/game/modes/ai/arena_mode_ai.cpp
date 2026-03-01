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

#include "game/detail/pathfinding/navigate_pathfinding.hpp"

#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/tasks/find_closest_enemy.hpp"
#include "game/modes/ai/tasks/interpolate_crosshair.hpp"
#include "game/modes/ai/tasks/handle_purchases.hpp"
#include "game/modes/ai/tasks/listen_for_footsteps.hpp"
#include "game/modes/ai/tasks/ai_behavior_tree.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"
#include "game/modes/ai/behaviors/eval_behavior_tree.hpp"
#include "game/modes/ai/behaviors/ai_behavior_process_ctx.hpp"
#include "game/modes/ai/behaviors/ai_behavior_patrol_process.hpp"
#include "game/modes/ai/behaviors/ai_behavior_defuse_process.hpp"
#include "game/modes/ai/behaviors/ai_behavior_retrieve_bomb_process.hpp"
#include "game/modes/ai/behaviors/ai_behavior_plant_process.hpp"
#include "game/modes/ai/intents/calc_pathfinding_request.hpp"
#include "game/modes/ai/intents/calc_movement_and_crosshair.hpp"
#include "game/modes/ai/intents/calc_wielding_intent.hpp"
#include "game/modes/ai/intents/calc_assigned_waypoint.hpp"
#include "game/modes/ai/intents/calc_movement_flags.hpp"
#include "game/modes/ai/intents/calc_requested_interaction.hpp"
#include "game/modes/ai/intents/calc_hand_flags.hpp"
#include "game/modes/ai/tasks/can_weapon_penetrate.hpp"
#include "game/cosmos/make_physics_path_hints.h"

arena_ai_result update_arena_mode_ai(
	cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta,
	const entity_id controlled_character_id,
	const mode_player_id& bot_player_id,
	const faction_type bot_faction,
	const money_type money,
	const bool is_ffa,
	xorshift_state& stable_round_rng,
	const difficulty_type difficulty,
	const cosmos_navmesh& navmesh,
	const bool bomb_planted,
	const entity_id bomb_entity,
	pathfinding_context* pathfinding_ctx,
	bool in_buy_area,
	const bool is_freeze_time
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
	const auto global_time_secs = static_cast<real32>(cosm.get_total_seconds_passed());
	const auto& physics = cosm.get_solvable_inferred().physics;

	const auto ctx = ai_character_context{
		ai_state,
		character_pos,
		physics,
		cosm,
		character_handle
	};

	AI_LOG("=== update_arena_mode_ai ===");

	/*
		===========================================================================
		PHASE 0: Ensure team-level and per-bot bombsite data is initialized.
		This runs on a per-bot basis but updates team state if needed.
		===========================================================================
	*/

	{
		const bool is_resistance = (bot_faction == faction_type::RESISTANCE);
		const bool is_metropolis = (bot_faction == faction_type::METROPOLIS);

		/*
			Resistance: if team has no chosen_bombsite yet, pick one randomly.
		*/
		if (is_resistance && team_state.chosen_bombsite == marker_letter_type::COUNT) {
			team_state.chosen_bombsite = ::choose_random_bombsite(arena_meta, stable_rng);
		}

		/*
			Per-bot: if patrol_letter is unset, assign the least-covered bombsite letter.
		*/
		if (ai_state.patrol_letter == marker_letter_type::COUNT) {
			if (is_metropolis) {
				ai_state.patrol_letter = ::find_least_assigned_bombsite(cosm, team_state, arena_meta);
			}
			else {
				const auto available = arena_meta.get_available_bombsite_letters();
				if (!available.empty()) {
					ai_state.patrol_letter = stable_rng.rand_element(available);
				}
				else {
					ai_state.patrol_letter = marker_letter_type::A;
				}
			}
		}
	}

	/*
		===========================================================================
		PHASE 1: Update combat target tracking (before behavior tree evaluation).
		NOTE: listen_for_footsteps is now called in post_solve_arena_mode_ai.
		===========================================================================
	*/

	/* Check for visible enemies and update combat_target. */
	const bool is_camping = ::is_camping_on_waypoint(ai_state.last_behavior);
	const auto closest_enemy = ::find_closest_enemy(ctx, is_ffa, is_camping);
	const bool sees_target = closest_enemy.is_set();

	if (sees_target) {
		/*
			Could happen at the beginning of the round,
			prevent any buying logic in that case.
		*/
		in_buy_area = false;

		const auto enemy_handle = cosm[closest_enemy];
		const auto enemy_pos = enemy_handle.get_logic_transform().pos;
		ai_state.combat_target.acquire_target_seen(
			stable_rng,
			global_time_secs,
			closest_enemy,
			enemy_pos,
			character_pos
		);
	}
	else {
		/*
			Target can be lost: if we're in combat and have already dashed to last_known_pos
			but still don't see the target, we've reached the last known position and the
			enemy is gone. Clear the combat target.
		*/
		if (auto* combat = ::get_behavior_if<ai_behavior_combat>(ai_state.last_behavior)) {
			if (combat->has_dashed_for_known_position(ai_state.combat_target.last_known_pos)) {
				ai_state.combat_target.clear();
			}
		}
	}

	/*
		===========================================================================
		PHASE 2: Evaluate behavior tree to get desired behavior.
		===========================================================================
	*/

	const auto round_state = ai_round_state{
		bomb_planted,
		bomb_entity,
		global_time_secs
	};

	const auto desired_behavior = ::eval_behavior_tree(
		cosm,
		ai_state,
		team_state,
		bot_player_id,
		controlled_character_id,
		bot_faction,
		character_pos,
		round_state,
		stable_rng
	);

	/*
		Check if behavior changed - if so, handle transition.
	*/
	if (desired_behavior.index() != ai_state.last_behavior.index()) {
		AI_LOG("Behavior changed - transitioning");
		::behavior_state_transition(
			ai_state.last_behavior,
			desired_behavior,
			ai_state
		);
		ai_state.last_behavior = desired_behavior;
	}

	/*
		===========================================================================
		PHASE 2.5: Calculate target_acquired for shooting,
	    as well as for shooting	through walls.
		===========================================================================
	*/

	bool target_acquired = false;
	std::optional<vec2> target_enemy_velocity = std::nullopt;

	if (::is_behavior<ai_behavior_combat>(ai_state.last_behavior)) {
		if (ai_state.combat_target.active(cosm, global_time_secs)) {
			const auto time_since_known = global_time_secs - ai_state.combat_target.when_last_known_secs;
			const auto shoot_wall_time_limit = ai_state.combat_target.chosen_combat_time_secs / 20.0f;

			/* 
				This will always be true when seeing the enemy,
				because then when_last_known_secs will is always updated to now.
			*/
			if (time_since_known < shoot_wall_time_limit) {
				target_acquired = ::can_weapon_penetrate(character_handle, ai_state.combat_target.last_known_pos);

				if (sees_target) {
					if (const auto enemy_handle = cosm[closest_enemy]) {
						target_enemy_velocity = enemy_handle.get_effective_velocity();
					}
				}
			}
		}
	}

	/*
		The target position is always last_known_pos from combat_target.
		This works for both seen (updated each frame) and wall penetration cases.
	*/
	const auto target_enemy_pos = ai_state.combat_target.last_known_pos;

	/*
		===========================================================================
		PHASE 3: Calculate movement direction FIRST (to capture path_completed).
		
		IMPORTANT: We must calculate movement direction BEFORE checking pathfinding
		request changes. This ensures we capture path_completed=true before the
		pathfinding state gets cleared when the request changes.
		===========================================================================
	*/

	const auto move_result = ::calc_movement_and_crosshair(
		ai_state.last_behavior,
		ai_state.pathfinding,
		character_pos,
		navmesh,
		character_handle,
		dt_secs,
		cosm,
		bomb_entity,
		target_acquired,
		target_enemy_pos,
		target_enemy_velocity
	);

	/*
		===========================================================================
		PHASE 4: Calculate pathfinding request (after movement calculation).
		===========================================================================
	*/

	const auto new_request = ::calc_current_pathfinding_request(
		cosm,
		ai_state.last_behavior,
		ai_state.combat_target,
		team_state,
		arena_meta,
		bot_player_id,
		character_pos,
		bomb_planted,
		bomb_entity,
		global_time_secs,
		navmesh,
		stable_rng
	);

	/* Check if request changed - reinitialize pathfinding. */
	if (new_request != ai_state.current_pathfinding_request) {
		AI_LOG("Pathfinding request changed - reinitializing");

		ai_state.current_pathfinding_request = new_request;
		ai_state.clear_pathfinding();

		if (new_request != std::nullopt) {
			const auto physics_hints = make_physics_path_hints(cosm);
			ai_state.pathfinding = ::start_pathfinding_to(character_pos, new_request->target, navmesh, &physics_hints, pathfinding_ctx);

			if (ai_state.is_pathfinding_active()) {
				ai_state.pathfinding->exact_destination = new_request->exact;
			}
		}
	}

	AI_LOG_NVPS(move_result.is_navigating, move_result.path_completed, move_result.can_sprint);

	/*
		===========================================================================
		PHASE 5: Process current behavior (call process() on last_behavior via std::visit).
		All behaviors receive the same context struct.
		===========================================================================
	*/

	{
		auto process_ctx = ai_behavior_process_ctx{
			cosm,
			step,
			ai_state,
			team_state,
			bot_player_id,
			bot_faction,
			controlled_character_id,
			character_pos,
			dt_secs,
			global_time_secs,
			stable_rng,
			bomb_entity,
			bomb_planted,
			move_result.path_completed
		};

		auto process_lbd = [&process_ctx](auto& behavior) {
			behavior.process(process_ctx);
		};

		std::visit(process_lbd, ai_state.last_behavior);
	}

	/*
		===========================================================================
		PHASE 6: Calculate and apply weapon wielding (via intent calculator).
		During freeze time, don't holster so bots show their weapons.
		===========================================================================
	*/

	/*
		If in buy area and not done buying, stay still to make purchases.
	*/
	const bool is_thinking_what_to_buy = in_buy_area && !ai_state.already_nothing_more_to_buy;

	const auto wielding_intent = ::calc_wielding_intent(
		ai_state.last_behavior,
		character_handle,
		move_result.nearing_end,
		is_freeze_time,
		is_thinking_what_to_buy
	);

	if (wielding_intent.should_change) {
		::perform_wielding(step, character_handle, wielding_intent.desired_wielding);
	}

	/*
		===========================================================================
		PHASE 6.5: Calculate and apply requested interactions (via intent calculator).
		===========================================================================
	*/

	{
		const auto requested_interactions = ::calc_requested_interaction(ai_state.last_behavior);

		if (auto* sentience = character_handle.find<components::sentience>()) {
			sentience->requested_interactions = requested_interactions;
		}
	}

	/*
		===========================================================================
		PHASE 7: Apply movement, aiming, crosshair, purchases.
		===========================================================================
	*/

	movement.flags.walking = ::should_walk_silently(ai_state.last_behavior);
	movement.flags.sprinting = ::should_sprint(ai_state.last_behavior, move_result.can_sprint);
	movement.flags.dashing = ::should_dash_for_combat(ai_state.last_behavior, ai_state.combat_target, character_pos);

	if (auto* sentience = character_handle.find<components::sentience>()) {
		auto& consciousness = sentience->get<consciousness_meter_instance>();

		if (ai_state.stamina_cooldown) {
			if (consciousness.value < 10) {
				ai_state.stamina_cooldown = true;
			}
		}
		else {
			if (consciousness.value > 100) {
				ai_state.stamina_cooldown = false;
			}
		}

		if (ai_state.stamina_cooldown) {
			movement.flags.sprinting = false;
		}
	}

	if (is_thinking_what_to_buy) {
		movement.flags.set_from_closest_direction(vec2::zero);
	}
	else if (move_result.movement_direction.has_value()) {
		movement.flags.set_from_closest_direction(*move_result.movement_direction);
	}
	else {
		movement.flags.set_from_closest_direction(vec2::zero);
	}

	AI_LOG_NVPS(movement.flags.walking, movement.flags.sprinting, movement.flags.dashing);

	/*
		Calculate and apply hand_flags (triggers, planting).
	*/
	{
		const auto hand_flags = ::calc_hand_flags(
			ai_state.last_behavior,
			target_acquired,
			target_enemy_pos,
			character_pos,
			character_handle
		);

		if (auto* sentience = character_handle.find<components::sentience>()) {
			sentience->hand_flags[0] = hand_flags.hand_flag_0;
			sentience->hand_flags[1] = hand_flags.hand_flag_1;

			if (const auto crosshair = character_handle.find_crosshair()) {
				const auto amount = repro::fabs(crosshair->recoil.rotation);

				if (ai_state.recoil_cooldown) {
					if (amount < 0.5f) {
						ai_state.recoil_cooldown = false;
					}

				}
				else {
					if (amount > 5.5f) {
						ai_state.recoil_cooldown = true;
					}
				}
			}

			if (ai_state.recoil_cooldown) {
				sentience->hand_flags[0] = false;
				sentience->hand_flags[1] = false;
			}
		}
	}

	if (!is_thinking_what_to_buy) {
		::interpolate_crosshair(ctx, move_result.crosshair_offset, target_acquired, dt_secs, difficulty, move_result.is_navigating);
	}

	arena_ai_result result;

	/*
		Only attempt purchases when in buy area.
	*/
	if (in_buy_area) {
		result.item_purchase = ::handle_purchases(ctx, money, dt_secs, stable_rng);
	}

	if (move_result.path_completed) {
		ai_state.current_pathfinding_request = std::nullopt;
		ai_state.clear_pathfinding();
	}

	return result;
}

void post_solve_arena_mode_ai(
	cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	const entity_id controlled_character_id,
	const bool is_ffa,
	const bool bomb_planted
) {
	const auto character_handle = cosm[controlled_character_id];

	if (!character_handle.alive()) {
		return;
	}

	const auto bot_faction = character_handle.get_official_faction();
	const auto character_pos = character_handle.get_logic_transform().pos;
	const auto global_time_secs = cosm.get_total_seconds_passed();
	const auto& physics = cosm.get_solvable_inferred().physics;

	const auto ctx = ai_character_context{
		ai_state,
		character_pos,
		physics,
		cosm,
		character_handle
	};

	/*
		Listen for footsteps - must be done in post_solve since footstep sounds are posted here.
	*/
	::listen_for_footsteps(ctx, step, is_ffa, global_time_secs, bomb_planted);

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
	const auto filter = predefined_queries::line_of_sight();

	auto rng_state = xorshift_state{ static_cast<uint64_t>(global_time_secs * 1000.0f + 12345) };
	auto rng = randomization(rng_state);

	auto is_enemy_faction = [&](const faction_type shooter_faction) {
		if (is_ffa) {
			return bot_faction != shooter_faction;
		}

		return bot_faction != shooter_faction && shooter_faction != faction_type::SPECTATOR;
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
			/*
				Gunshot seen - update combat target.
			*/
			ai_state.combat_target.acquire_target_seen(
				rng,
				global_time_secs,
				shot.capability,
				muzzle_pos,
				character_pos
			);
		}
		else {
			/*
				No direct line of sight. Check if we can penetrate walls to shoot them.
				If weapon can penetrate to the muzzle position, full_acquire.
			*/
			if (::can_weapon_penetrate(character_handle, muzzle_pos)) {
				ai_state.combat_target.full_acquire(
					rng,
					global_time_secs,
					shot.capability,
					muzzle_pos
				);
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

		/*
			Damage always initiates combat (no LoS check needed).
			Use full_acquire since damage is always significant.
		*/
		ai_state.combat_target.full_acquire(
			rng,
			global_time_secs,
			attacker_id,
			attacker_pos
		);
	}
}
