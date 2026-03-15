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

void update_arena_mode_ai_team(
	cosmos& cosm,
	arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta,
	std::map<mode_player_id, arena_mode_player>& players,
	const faction_type faction,
	const bool bomb_planted,
	randomization& rng
) {
	/*
		Resistance: if team has no chosen_bombsite yet, pick one randomly.
	*/
	if (faction == faction_type::RESISTANCE && team_state.chosen_bombsite == marker_letter_type::COUNT) {
		team_state.chosen_bombsite = ::choose_random_bombsite(arena_meta, rng);
	}

	/*
		Metropolis: rebalance patrol_letter distribution once per step.
		If any bot's letter is over-covered by 2+ compared to the least-covered,
		switch that bot to the least-covered letter.
	*/
	if (faction == faction_type::METROPOLIS && !bomb_planted) {
		const auto least = ::find_least_assigned_bombsite(cosm, team_state, arena_meta);
		const auto most = ::find_most_assigned_bombsite(cosm, team_state, arena_meta);

		if (most.count >= least.count + 2 && most.example_bot.is_set()) {
			for (auto& bot : only_bot(players)) {
				if (bot.first == most.example_bot) {
					bot.second.ai_state.patrol_letter = least.letter;

					if (auto* patrol = ::get_behavior_if<ai_behavior_patrol>(bot.second.ai_state.last_behavior)) {
						*patrol = ai_behavior_patrol();
					}

					break;
				}
			}
		}
	}
}

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

	/*
		===========================================================================
		PHASE 0: Per-bot patrol_letter initialization.
		Team-level data (chosen_bombsite, rebalancing) is handled by
		update_arena_mode_ai_team called before the per-bot loop.
		===========================================================================
	*/

	if (bot_faction == faction_type::METROPOLIS && ai_state.patrol_letter == marker_letter_type::COUNT) {
		ai_state.patrol_letter = ::find_least_assigned_bombsite(cosm, team_state, arena_meta).letter;
	}

	/*
		Resistance bots all go to the team's chosen bombsite — there is no
		per-bot letter split.  Always keep patrol_letter in sync so a
		mid-round change (e.g. bomb carrier realigning to nearest bombsite)
		is picked up immediately by every bot.
	*/
	if (bot_faction == faction_type::RESISTANCE && team_state.chosen_bombsite != marker_letter_type::COUNT) {
		if (ai_state.patrol_letter != team_state.chosen_bombsite) {
			ai_state.patrol_letter = team_state.chosen_bombsite;

			/* Clear cached patrol waypoint so the bot reroutes to the new site. */
			if (auto* patrol = ::get_behavior_if<ai_behavior_patrol>(ai_state.last_behavior)) {
				patrol->patrol_waypoint = entity_id::dead();
			}
		}
	}

	/*
		===========================================================================
		PHASE 0.5: Commit any pending reaction-time alerts.
		===========================================================================
	*/

	ai_state.alertness.base_rt_secs = ::get_reaction_time_secs(difficulty);

	/* Commit LOS change alert (dedicated slot, never evicted). */
	if (ai_state.alertness.is_los_change_ready(global_time_secs)) {
		const auto& alert = *ai_state.alertness.los_change_alert;

		if (alert.enemy.is_set()) {
			const auto enemy_handle = cosm[alert.enemy];

			if (enemy_handle.alive() && sentient_and_conscious(enemy_handle)) {
				ai_state.confirmed_closest_enemy = alert.enemy;

				ai_state.combat_target.acquire_target_seen(
					stable_rng,
					global_time_secs,
					alert.enemy,
					alert.enemy_pos,
					character_pos
				);
			}
			else {
				ai_state.confirmed_closest_enemy = {};
			}
		}
		else {
			ai_state.confirmed_closest_enemy = {};
		}

		ai_state.alertness.los_change_alert.reset();
	}

	/* Commit regular alerts (damage, gunshots, footsteps). */
	{
		const auto ready_idx = ai_state.alertness.find_ready_index(global_time_secs);

		if (ready_idx.has_value()) {
			const auto alert = ai_state.alertness.alerts[*ready_idx];
			ai_state.alertness.remove_at(*ready_idx);

			const auto enemy_handle = cosm[alert.enemy];

			if (enemy_handle.alive() && sentient_and_conscious(enemy_handle)) {
				switch (alert.acquire_type) {
					case alert_acquire_type::FULL:
						ai_state.combat_target.full_acquire(
							stable_rng,
							global_time_secs,
							alert.enemy,
							alert.enemy_pos
						);
						break;
					case alert_acquire_type::SEEN:
					case alert_acquire_type::CLOSEST_LOS_CHANGE:
						ai_state.combat_target.acquire_target_seen(
							stable_rng,
							global_time_secs,
							alert.enemy,
							alert.enemy_pos,
							character_pos
						);
						break;
					case alert_acquire_type::HEARD_ONLY:
						ai_state.combat_target.acquire_target_heard(
							global_time_secs,
							alert.enemy,
							alert.enemy_pos
						);
						break;
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

	const bool is_camping = ::is_camping_on_waypoint(ai_state.last_behavior);
	const auto now_closest_enemy = ::find_closest_enemy(ctx, is_ffa, is_camping);

	LOG_NVPS(now_closest_enemy);
	if (now_closest_enemy != ai_state.confirmed_closest_enemy) {
		/* World state differs from bot's perception — queue/update LOS change. */
		const auto enemy_pos = now_closest_enemy.is_set()
			? cosm[now_closest_enemy].get_logic_transform().pos
			: vec2::zero;

		LOG("Q LOS CHANGE");
		ai_state.alertness.queue_los_change(now_closest_enemy, enemy_pos, global_time_secs);
	}
	else {
		if (const auto enemy_handle = cosm[ai_state.confirmed_closest_enemy]) {
			/* Confirmed visual contact with same target — immediate position update. */
			const auto enemy_pos = enemy_handle.get_logic_transform().pos;

			ai_state.combat_target.acquire_target_seen(
				stable_rng,
				global_time_secs,
				ai_state.confirmed_closest_enemy,
				enemy_pos,
				character_pos
			);
		}
	}

	/*
		All downstream logic uses the confirmed (reaction-time-delayed) state,
		never the raw find_closest_enemy result.
	*/
	const bool sees_target = ai_state.confirmed_closest_enemy.is_set();

	if (sees_target) {
		/*
			Could happen at the beginning of the round,
			prevent any buying logic in that case.
		*/
		in_buy_area = false;
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
		arena_meta,
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
		AI_LOG("Behavior changed - transitioning (from index %x to index %x, bomb_planted=%x, faction=%x)",
			ai_state.last_behavior.index(), desired_behavior.index(),
			bomb_planted, static_cast<int>(bot_faction));
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
			if (ai_state.confirmed_closest_enemy.is_set()) {
				/* Confirmed visual contact — direct engagement. */
				target_acquired = ::can_weapon_penetrate(character_handle, ai_state.combat_target.last_known_pos);

				if (const auto enemy_handle = cosm[ai_state.confirmed_closest_enemy]) {
					target_enemy_velocity = enemy_handle.get_effective_velocity();
				}
			}
			else {
				/* Not confirmed seeing target — wall penetration at last known position. */
				const auto time_since_known = global_time_secs - ai_state.combat_target.when_last_known_secs;
				const auto shoot_wall_time_limit = ai_state.combat_target.chosen_combat_time_secs / 20.0f;
				LOG_NVPS(time_since_known, shoot_wall_time_limit);

				if (time_since_known < shoot_wall_time_limit) {
					target_acquired = ::can_weapon_penetrate(character_handle, ai_state.combat_target.last_known_pos);
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
			else {
				/* Unconditional LOG (not AI_LOG) — always visible even with LOG_AI=0. */
				LOG("AI ERROR: start_pathfinding_to FAILED for bot at (%x,%x) -> target (%x,%x). Waypoint unreachable?",
					character_pos.x, character_pos.y, new_request->target.pos.x, new_request->target.pos.y);
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
		::interpolate_crosshair(ctx, move_result.crosshair_offset, dt_secs, difficulty, move_result.is_navigating);
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

		/* Only react to gunshots within hearing range (80% of max_distance) */
		{
			bool out_of_range = false;
			const auto gun_handle = cosm[shot.subject];

			if (gun_handle.alive()) {
				gun_handle.dispatch_on_having_all<invariants::gun>([&](const auto& typed_gun) {
					const auto& gun_def = typed_gun.template get<invariants::gun>();
					const auto max_dist = gun_def.muzzle_shot_sound.modifier.max_distance;

					if (max_dist > 0.f) {
						const auto hearing_dist = max_dist * 0.8f;
						const auto dist_sq = (character_pos - muzzle_pos).length_sq();

						if (dist_sq > hearing_dist * hearing_dist) {
							out_of_range = true;
						}
					}
				});
			}

			if (out_of_range) {
				continue;
			}
		}

		const auto raycast = physics.ray_cast_px(
			cosm.get_si(),
			character_pos,
			muzzle_pos,
			filter,
			character_handle
		);

		LOG_NVPS(raycast.hit, ::can_weapon_penetrate(shooter, character_pos));
		if (!raycast.hit || ::can_weapon_penetrate(shooter, character_pos)) {
			LOG("QUEUE");
			/*
				Gunshot muzzle flash seen — queue through reaction time.
			*/
			ai_state.alertness.queue_alert({
				shot.capability,
				muzzle_pos,
				static_cast<real32>(global_time_secs),
				0.8f,
				alert_acquire_type::SEEN
			});
		}
	}

	/*
		Damage-based combat initiation.
		Receiving damage initiates COMBAT regardless of LoS,
		but with a shorter reaction delay (0.5x multiplier).
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

		ai_state.alertness.queue_alert({
			attacker_id,
			attacker_pos,
			static_cast<real32>(global_time_secs),
			0.5f,
			alert_acquire_type::FULL
		});
	}
}

real32 get_reaction_time_secs(const difficulty_type difficulty) {
	switch (difficulty) {
		case difficulty_type::EASY:   return 1.0f;
		case difficulty_type::MEDIUM: return 0.50f;
		case difficulty_type::HARD:   return 0.35f;
		default:                      return 0.20f;
	}
}
