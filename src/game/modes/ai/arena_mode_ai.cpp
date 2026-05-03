#include "game/components/movement_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/gun_component.h"
#include "game/components/sentience_component.h"
#include "game/components/hand_fuse_component.h"
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
#include "game/detail/pathfinding/pathfinding.h"
#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/inventory/wielding_setup.hpp"
#include "game/detail/inventory/perform_wielding.hpp"
#include <limits>

#include "game/detail/path_navigation/navigate_path.hpp"

#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/tasks/find_closest_enemy.hpp"
#include "game/modes/ai/tasks/interpolate_crosshair.hpp"
#include "game/modes/ai/tasks/handle_purchases.hpp"
#include "game/modes/ai/tasks/listen_for_sound_cues.hpp"
#include "game/modes/ai/tasks/find_best_weapon.hpp"
#include "game/modes/ai/tasks/line_of_sight.hpp"
#include "game/modes/ai/intents/calc_movement_flags.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"
#include "game/modes/ai/behaviors/eval_behavior_tree.hpp"
#include "game/modes/ai/behaviors/ai_behavior_process_ctx.hpp"
#include "game/modes/ai/behaviors/ai_behavior_patrol_process.hpp"
#include "game/modes/ai/behaviors/ai_behavior_defuse_process.hpp"
#include "game/modes/ai/behaviors/ai_behavior_retrieve_bomb_process.hpp"
#include "game/modes/ai/behaviors/ai_behavior_plant_process.hpp"
#include "game/modes/ai/intents/calc_navigation_request.hpp"
#include "game/modes/ai/intents/calc_movement_and_crosshair.hpp"
#include "game/modes/ai/intents/calc_wielding_intent.hpp"
#include "game/modes/ai/intents/calc_assigned_waypoint.hpp"
#include "game/modes/ai/intents/calc_movement_flags.hpp"
#include "game/modes/ai/intents/calc_requested_interaction.hpp"
#include "game/modes/ai/intents/calc_hand_flags.hpp"
#include "game/modes/ai/tasks/can_weapon_penetrate.hpp"
#include "game/cosmos/make_physics_path_hints.h"
#include "game/modes/ai/tasks/bot_avoidance.hpp"
#include "game/modes/ai/tasks/bullet_avoidance.hpp"
#include "game/modes/ai/tasks/danger_avoidance.hpp"
#include "game/modes/ai/tasks/take_cover.hpp"

void update_arena_mode_ai_team(
	cosmos& cosm,
	arena_mode_ai_team_state& team_state,
	const arena_mode_ai_arena_meta& arena_meta,
	std::map<mode_player_id, arena_mode_player>& players,
	const faction_type faction,
	const bool bomb_planted,
	randomization& rng,
	const cosmos_navmesh& navmesh,
	const entity_id bomb_entity,
	pathfinding_context* pathfinding_ctx
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

		if (most.count >= least.count + 2 && sentient_and_conscious(cosm[most.example_bot])) {
			for (auto& bot : only_bot(players)) {
				if (bot.second.controlled_character_id == most.example_bot) {
					bot.second.ai_state.patrol_letter = least.letter;

					if (auto* patrol = ::get_behavior_if<ai_behavior_patrol>(bot.second.ai_state.last_behavior)) {
						*patrol = ai_behavior_patrol();
					}

					break;
				}
			}
		}
	}

	/*
		Metropolis: when bomb is planted, select the bot with the shortest path to
		the bomb as the defuser — but only if the current defuser is dead/unconscious
		(or none assigned).  Non-combat bots are preferred candidates; if all alive
		conscious bots are in combat, all of them are considered so the closest one
		can still take the mission.
	*/
	if (faction == faction_type::METROPOLIS && bomb_planted) {
		if (!sentient_and_conscious(cosm[team_state.bot_with_defuse_mission])) {
			struct candidate_t {
				entity_id char_id;
				arena_mode_player* player_ptr = nullptr;
			};

			const auto gather_alive_conscious_bots = [&](const bool include_combat) {
				std::vector<candidate_t> result;

				for (auto& bot : only_bot(players)) {
					if (bot.second.get_faction() != faction_type::METROPOLIS) {
						continue;
					}

					const auto char_id = bot.second.controlled_character_id;

					if (!sentient_and_conscious(cosm[char_id])) {
						continue;
					}

					if (!include_combat && ::is_behavior<ai_behavior_combat>(bot.second.ai_state.last_behavior)) {
						continue;
					}

					result.push_back({ char_id, &bot.second });
				}

				return result;
			};

			/* Preferred: alive conscious non-combat bots. */
			auto candidates = gather_alive_conscious_bots(false);

			/*
				Fallback: if every alive conscious bot is in combat, consider all of them
				so the closest one to the bomb can still take the defuse mission.
			*/
			if (candidates.empty()) {
				candidates = gather_alive_conscious_bots(true);
			}

			/*
				After assigning the defuse mission, clear the bot's combat target if it is
				currently in combat — otherwise update_arena_mode_ai will see ai_behavior_combat
				as last_behavior in the same tick and immediately unassign the mission.
			*/
			auto assign_defuser = [&](entity_id char_id, arena_mode_player* player_ptr) {
				team_state.bot_with_defuse_mission = char_id;

				if (::is_behavior<ai_behavior_combat>(player_ptr->ai_state.last_behavior)) {
					player_ptr->ai_state.combat_target.clear();
				}
			};

			if (candidates.size() == 1) {
				const bool is_in_combat = ::is_behavior<ai_behavior_combat>(candidates[0].player_ptr->ai_state.last_behavior);

				if (!is_in_combat) {
					assign_defuser(candidates[0].char_id, candidates[0].player_ptr);
				}
			}
			else if (candidates.size() > 1) {
				const auto bomb_handle = cosm[bomb_entity];

				if (bomb_handle.alive()) {
					const auto bomb_pos = bomb_handle.get_logic_transform().pos;

					entity_id best_char_id;
					arena_mode_player* best_player_ptr = nullptr;
					real32 best_path_len = std::numeric_limits<real32>::max();

					for (auto& cand : candidates) {
						const auto char_handle = cosm[cand.char_id];

						if (!char_handle.alive()) {
							continue;
						}

						const auto bot_pos = char_handle.get_logic_transform().pos;
						const auto paths = ::find_path_across_islands_many_full(navmesh, bot_pos, bomb_pos, nullptr, pathfinding_ctx);

						if (paths.empty()) {
							continue;
						}

						real32 path_len = 0.f;

						for (const auto& path : paths) {
							const auto& island = navmesh.islands[path.island_index];

							for (std::size_t i = 1; i < path.nodes.size(); ++i) {
								const auto a = ::cell_to_world(island, path.nodes[i - 1].cell_xy);
								const auto b = ::cell_to_world(island, path.nodes[i].cell_xy);
								path_len += (b - a).length();
							}
						}

						if (path_len < best_path_len) {
							best_path_len = path_len;
							best_char_id = cand.char_id;
							best_player_ptr = cand.player_ptr;
						}
					}

					if (best_char_id.is_set()) {
						assign_defuser(best_char_id, best_player_ptr);
					}
				}
			}
		}
	}

	/*
		Metropolis: when the bomb is first planted, read the bombsite letter from the fuse
		and steer all living Metropolis bots to patrol that site.
		Only done once per round (guarded by bomb_plant_handled).
	*/
	if (faction == faction_type::METROPOLIS && bomb_planted && !team_state.bomb_plant_handled) {
		if (const auto bomb_handle = cosm[bomb_entity]) {
			if (const auto* fuse = bomb_handle.find<components::hand_fuse>()) {
				if (fuse->bombsite_letter != static_cast<uint8_t>(-1)) {
					const auto planted_letter = static_cast<marker_letter_type>(fuse->bombsite_letter);

					for (auto& bot : only_bot(players)) {
						if (bot.second.get_faction() != faction_type::METROPOLIS) {
							continue;
						}

						bot.second.ai_state.patrol_letter = planted_letter;

						if (auto* patrol = ::get_behavior_if<ai_behavior_patrol>(bot.second.ai_state.last_behavior)) {
							*patrol = ai_behavior_patrol();
						}
					}

					team_state.bomb_plant_handled = true;
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
	const bool is_freeze_time,
	const std::size_t bot_index,
	const std::size_t num_bots
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

	const auto* const bot_sentience = character_handle.find<components::sentience>();

	/*
		Flash/stun thresholds — blind/deafen the bot when hit by a flashbang.
		Blinded: visual detection and LoS updates are frozen (last known state is kept).
		Deafened: footstep processing is skipped in post_solve.
	*/
	const bool is_blinded  = bot_sentience != nullptr && bot_sentience->visual_flash_secs > 0.5f;
	const bool is_deafened = bot_sentience != nullptr && bot_sentience->audio_flash_secs  > 2.0f;

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
	if (
		bot_faction == faction_type::RESISTANCE &&
		team_state.chosen_bombsite != marker_letter_type::COUNT &&
		!ai_state.escaping_explosion
	) {
		if (ai_state.patrol_letter != team_state.chosen_bombsite) {
			ai_state.patrol_letter = team_state.chosen_bombsite;

			/* Clear cached patrol waypoint so the bot reroutes to the new site. */
			if (auto* patrol = ::get_behavior_if<ai_behavior_patrol>(ai_state.last_behavior)) {
				patrol->patrol_waypoint = entity_id::dead();
			}
		}
	}

	/*
		RESISTANCE escape: when the bomb has <= 6 seconds until explosion,
		change patrol_letter to a different bombsite so the bot runs away.
		If only one bombsite exists, set patrol_letter to COUNT so that
		patrol_process uses source_spawn_point as fallback.
	*/

	if (
		bot_faction == faction_type::RESISTANCE &&
		bomb_planted &&
		!ai_state.escaping_explosion &&
		bomb_entity.is_set()
	) {
		if (const auto bomb_handle = cosm[bomb_entity]) {
			if (const auto* fuse = bomb_handle.find<components::hand_fuse>()) {
				if (fuse->armed()) {
					const auto& clk = cosm.get_clock();
					const auto remaining_ms = clk.get_remaining_ms(fuse->fuse_delay_ms, fuse->when_armed);

					if (remaining_ms <= 6000.0f) {
						ai_state.escaping_explosion = true;

						const auto available = arena_meta.get_available_bombsite_letters();

						if (available.size() > 1) {
							auto candidates = available;

							for (auto it = candidates.begin(); it != candidates.end(); ++it) {
								if (*it == ai_state.patrol_letter) {
									candidates.erase(it);
									break;
								}
							}

							if (!candidates.empty()) {
								ai_state.patrol_letter = candidates[stable_rng.randval(0u, static_cast<unsigned>(candidates.size() - 1))];
							}
						}
						else {
							/*
								Only one bombsite letter — no other site to flee to.
								Set patrol_letter to COUNT so patrol_process uses
								source_spawn_point as the escape destination.
							*/
							ai_state.patrol_letter = marker_letter_type::COUNT;
						}

						ai_state.last_behavior = ai_behavior_patrol();
					}
				}
			}
		}
	}

	/*
		===========================================================================
		PHASE 0.5: Commit any pending reaction-time alerts.
		===========================================================================
	*/

	ai_state.alertness.base_rt_secs = ::get_reaction_time_secs(difficulty);

	bool bot_is_bomb_carrier = false;

	if (!bomb_planted && bomb_entity.is_set()) {
		if (const auto bomb_handle = cosm[bomb_entity]) {
			bot_is_bomb_carrier = bomb_handle.get_owning_transfer_capability() == character_handle;
		}
	}

	/*
		When the bomb is planted, all Metropolis bots get defuser-style combat timing:
		engagement timeout measured from first contact so footstep spam cannot extend
		it indefinitely. This avoids ugly state transitions when the defuse mission
		changes hands mid-round.
	*/
	const bool defuse_soon = bomb_planted && bot_faction == faction_type::METROPOLIS;

	auto get_bomb_time_remaining = [&]() -> real32 {
		if (!bomb_planted || !bomb_entity.is_set()) {
			return 1000.0f;
		}

		if (const auto bomb_handle = cosm[bomb_entity]) {
			if (const auto* fuse = bomb_handle.find<components::hand_fuse>()) {
				if (fuse->armed()) {
					const auto& clk = cosm.get_clock();
					return clk.get_remaining_ms(fuse->fuse_delay_ms, fuse->when_armed) / 1000.0f;
				}
			}
		}

		return 1000.0f;
	};

	const auto bomb_time_remaining = get_bomb_time_remaining();
	constexpr auto CRITICAL_DEFUSE_TIME_SECS = 5.0f;
	const bool should_avoid_combat = (team_state.bot_with_defuse_mission == controlled_character_id) && bomb_time_remaining < CRITICAL_DEFUSE_TIME_SECS;

	/* Commit LOS change alert (dedicated slot, never evicted). */
	if (ai_state.alertness.is_los_change_ready(global_time_secs)) {
		if (is_blinded) {
			/* Blinded — discard the stale visual alert without committing. */
			ai_state.alertness.los_change_alert.reset();
		}
		else {
			const auto& alert = *ai_state.alertness.los_change_alert;

			if (alert.enemy.is_set()) {
				const auto enemy_handle = cosm[alert.enemy];

				if (enemy_handle.alive() && sentient_and_conscious(enemy_handle)) {
					ai_state.perceived_enemy = alert.enemy;

					ai_state.combat_target.on_visual_contact(
						stable_rng,
						global_time_secs,
						alert.enemy,
						alert.enemy_pos,
						character_pos,
						bot_is_bomb_carrier,
						defuse_soon,
						bomb_time_remaining,
						ai_state.alertness.base_rt_secs
					);
				}
				else {
					ai_state.perceived_enemy = {};
				}
			}
			else {
				ai_state.perceived_enemy = {};
			}

			ai_state.alertness.los_change_alert.reset();
		}
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
						ai_state.combat_target.force_engage(
							stable_rng,
							global_time_secs,
							alert.enemy,
							alert.enemy_pos,
							bot_is_bomb_carrier,
							defuse_soon,
							bomb_time_remaining,
							ai_state.alertness.base_rt_secs
						);
						break;
					case alert_acquire_type::SEEN:
					case alert_acquire_type::CLOSEST_LOS_CHANGE:
						if (!is_blinded) {
							ai_state.combat_target.on_visual_contact(
								stable_rng,
								global_time_secs,
								alert.enemy,
								alert.enemy_pos,
								character_pos,
								bot_is_bomb_carrier,
								defuse_soon,
								bomb_time_remaining,
								ai_state.alertness.base_rt_secs
							);
						}
						break;
					case alert_acquire_type::HEARD_ONLY:
						ai_state.combat_target.on_heard_footstep(
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
		PHASE 0.75: If bomb is planted and a METROPOLIS soldier is defusing,
		alert RESISTANCE soldiers about the defuser.
		===========================================================================
	*/

	if (bomb_planted && bot_faction == faction_type::RESISTANCE) {
		if (const auto bomb_handle = cosm[bomb_entity]) {
			if (const auto* fuse = bomb_handle.find<components::hand_fuse>()) {
				const auto defuser_handle = cosm[fuse->character_now_defusing];

				if (defuser_handle.alive()) {
					const auto defuser_pos = defuser_handle.get_logic_transform().pos;

					ai_state.alertness.queue_alert(ai_pending_alert{
						defuser_handle.get_id(),
						defuser_pos,
						global_time_secs,
						0.49f,
						alert_acquire_type::FULL
					});
				}
			}
		}
	}

	/*
		===========================================================================
		PHASE 0.875: If bomb is being planted by a RESISTANCE soldier, alert this
		METROPOLIS bot so it rushes to engage the planter.
		Only queued when the bot is not already in combat.
		===========================================================================
	*/

	if (
		!bomb_planted &&
		bot_faction == faction_type::METROPOLIS &&
		!::is_behavior<ai_behavior_combat>(ai_state.last_behavior) &&
		bomb_entity.is_set()
	) {
		if (const auto bomb_handle = cosm[bomb_entity]) {
			if (const auto* fuse = bomb_handle.find<components::hand_fuse>()) {
				if (fuse->when_started_arming.was_set() && !fuse->armed()) {
					const auto planter_handle = bomb_handle.get_owning_transfer_capability();

					if (planter_handle.alive() && sentient_and_conscious(planter_handle)) {
						const auto planter_pos = planter_handle.get_logic_transform().pos;

						ai_state.alertness.queue_alert(ai_pending_alert{
							planter_handle.get_id(),
							planter_pos,
							global_time_secs,
							1.0f,
							alert_acquire_type::FULL
						});
					}
				}
			}
		}
	}

	/*
		===========================================================================
		PHASE 1: Update combat target tracking (before behavior tree evaluation).
		NOTE: listen_for_sound_cues is now called in post_solve_arena_mode_ai.
		===========================================================================
	*/

	const bool is_camping = ::is_camping_on_waypoint(ai_state.last_behavior);
	const auto is_in_combat = [&]() {
		return ::is_behavior<ai_behavior_combat>(ai_state.last_behavior);
	};

	if (!is_blinded) {
		const auto detected_enemy = ::find_closest_enemy(ctx, is_ffa, is_camping, is_in_combat());

		if (detected_enemy != ai_state.perceived_enemy) {
			/* World state differs from bot's perception — queue/update LOS change. */
			const auto enemy_pos = detected_enemy.is_set()
				? cosm[detected_enemy].get_logic_transform().pos
				: vec2::zero;

			ai_state.alertness.queue_los_change(detected_enemy, enemy_pos, global_time_secs);
		}
		else {
			if (const auto enemy_handle = cosm[ai_state.perceived_enemy]) {
				/* Confirmed visual contact with same target — immediate position update. */
				const auto enemy_pos = enemy_handle.get_logic_transform().pos;

				ai_state.combat_target.on_visual_contact(
					stable_rng,
					global_time_secs,
					ai_state.perceived_enemy,
					enemy_pos,
					character_pos,
					bot_is_bomb_carrier,
					defuse_soon,
					bomb_time_remaining,
					ai_state.alertness.base_rt_secs
				);
			}
		}
	}

	/*
		All downstream logic uses the confirmed (reaction-time-delayed) state,
		never the raw find_closest_enemy result.
	*/
	const bool has_visual = ai_state.perceived_enemy.is_set();

	if (has_visual) {
		/*
			Could happen at the beginning of the round,
			prevent any buying logic in that case.
		*/
		in_buy_area = false;
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
		stable_rng,
		should_avoid_combat
	);

	/*
		Check if behavior changed - if so, handle transition.
	*/
	if (desired_behavior.index() != ai_state.last_behavior.index()) {
		AI_LOG(
			"Behavior changed - transitioning (from index %x to index %x, bomb_planted=%x, faction=%x)",
			ai_state.last_behavior.index(), desired_behavior.index(),
			bomb_planted, static_cast<int>(bot_faction)
		);

		::behavior_state_transition(
			ai_state.last_behavior,
			desired_behavior,
			ai_state
		);

		ai_state.last_behavior = desired_behavior;
	}

	/*
		If this bot was the defuser and just transitioned into combat,
		clear the defuse mission so the team update can assign another bot.
	*/
	if (bomb_planted && team_state.bot_with_defuse_mission == controlled_character_id) {
		if (::is_behavior<ai_behavior_combat>(ai_state.last_behavior)) {
			team_state.bot_with_defuse_mission = entity_id::dead();
		}
	}

	/*
		===========================================================================
		PHASE 2.5: Calculate target_acquired for shooting,
	    as well as for shooting	through walls.
		===========================================================================
	*/

	bool target_acquired = false;
	std::optional<vec2> aim_velocity = std::nullopt;

	if (::is_behavior<ai_behavior_combat>(ai_state.last_behavior)) {
		if (ai_state.combat_target.within_engagement_window(cosm, global_time_secs)) {
			if (ai_state.perceived_enemy.is_set()) {
				/*
					Confirmed visual contact — direct engagement.
					can_attack_position handles both gun (penetration) and melee/bare-hands
					(LoS-only); the swing/range decision is made downstream by calc_hand_flags.
				*/
				const auto enemy_handle = cosm[ai_state.perceived_enemy];

				target_acquired = ::can_attack_position(
					character_handle,
					enemy_handle,
					ai_state.combat_target.last_known_pos
				);

				if (enemy_handle) {
					aim_velocity = enemy_handle.get_effective_velocity();
				}
			}
			else {
				/* Not confirmed seeing target — wall penetration at last known position. */
				const auto time_since_known = global_time_secs - ai_state.combat_target.last_known_time_secs;
				const auto shoot_wall_time_limit = ai_state.combat_target.engagement_timeout_secs / 20.0f;

				if (time_since_known < shoot_wall_time_limit + ai_state.alertness.base_rt_secs) {
					const bool has_wall_between = !::los_to_any_vertices_of(
						character_handle,
						ai_state.combat_target.last_known_pos,
						physics,
						cosm.get_si(),
						predefined_queries::pathfinding()
					);

					if (has_wall_between) {
						/*
							Speculative shot through a wall: with no current LoS we can't
							verify the enemy moved, so assume they may still be hiding behind
							it — wall penetration is the only branch worth blind-firing on.
						*/
						target_acquired = ::can_weapon_penetrate(character_handle, ai_state.combat_target.last_known_pos);
					}
					else {
						/*
							Clear LoS to last_known_pos but no perceived_enemy this frame —
							the enemy must have moved (or we're facing away / awaiting reaction
							time). Don't blind-fire at empty space; let perceived_enemy
							re-acquire visually first.
						*/
					}
				}
			}
		}
	}

	/*
		The target position is always last_known_pos from combat_target.
		This works for both seen (updated each frame) and wall penetration cases.
	*/
	const auto aim_pos = ai_state.combat_target.last_known_pos;

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
		ai_state.navigation,
		character_pos,
		navmesh,
		character_handle,
		dt_secs,
		cosm,
		bomb_entity,
		target_acquired,
		aim_pos,
		aim_velocity
	);

	/*
		===========================================================================
		PHASE 3.5: Maintain searching_cover_pos for combat-without-visual.
		On every arrival (path_completed) while combat is still active and we
		have no visual on the target, pick a hiding spot via find_closest_cover
		using the previous cover (if any) as a secondary danger.  The cover is
		then read by calc_current_navigation_request below so the bot relocates
		from cover to cover until the engagement window closes.
		Cleared on visual reacquire so navigation falls back to last_known_pos.
		===========================================================================
	*/

	if (auto* combat = ::get_behavior_if<ai_behavior_combat>(ai_state.last_behavior)) {
		auto clear_cover = [&]() {
			combat->searching_cover_pos = std::nullopt;
			combat->last_known_time_at_cover_calc = 0.0f;
		};

		if (has_visual) {
			/*
				Visual reacquired — chase the (now-fresh) last_known_pos
				instead of hiding.
			*/
			clear_cover();
		}
		else {
			/*
				Heard a footstep or briefly saw the enemy: last_known_time_secs
				advanced past the moment cover was computed.  Abandon the
				stale cover so the bot investigates the new position first.
			*/
			if (
				combat->searching_cover_pos.has_value()
				&& ai_state.combat_target.last_known_time_secs > combat->last_known_time_at_cover_calc
			) {
				clear_cover();
			}

			/*
				Trigger when navigation finishes (normal arrival at
				last_known_pos or at the previous cover) or when the request
				is set but pathfinding never started — last_known_pos may be
				unreachable, so try cover from where we stand.
			*/
			const bool nav_failed_to_start =
				ai_state.current_navigation_request.has_value()
				&& !ai_state.is_navigating()
				&& !combat->searching_cover_pos.has_value()
			;

			const bool should_recalc =
				(move_result.path_completed || nav_failed_to_start)
				&& ai_state.combat_target.within_engagement_window(cosm, global_time_secs)
			;

			if (should_recalc) {
				const auto previous_cover = combat->searching_cover_pos;

				const auto cover_pos = ::find_closest_cover(
					navmesh,
					character_pos,
					ai_state.combat_target.last_known_pos,
					previous_cover,
					physics,
					cosm.get_si(),
					4000.0f
				);

				if (cover_pos.has_value()) {
					combat->searching_cover_pos = cover_pos;
					combat->last_known_time_at_cover_calc = ai_state.combat_target.last_known_time_secs;
				}
			}
		}
	}

	/*
		===========================================================================
		PHASE 4: Calculate navigation request (after movement calculation).
		===========================================================================
	*/

	const auto aim_radius_to_shoot = ::calc_aim_radius_to_shoot(character_handle);

	const bool has_direct_los_to_enemy = [&]() {
		if (!is_in_combat() || !ai_state.perceived_enemy.is_set()) {
			return false;
		}

		const auto enemy_handle = cosm[ai_state.perceived_enemy];

		if (!enemy_handle.alive()) {
			return false;
		}

		const auto filter = predefined_queries::pathfinding();
		const bool any_los = ::los_to_any_vertices_of(enemy_handle, character_pos, physics, cosm.get_si(), filter);
		return any_los;
	}();

	const auto new_request = ::calc_current_navigation_request(
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
		stable_rng,
		aim_radius_to_shoot,
		has_direct_los_to_enemy
	);

	/* Check if request changed - reinitialize navigation. */

	/*
		Danger avoidance and take-cover avoidance can override the effective
		navigation request. Priority: danger > take cover > normal target.
	*/
	const auto effective_request = [&]() -> std::optional<ai_navigation_request> {
		if (ai_state.avoided_bullet_timer >= 0.0f) {
			return std::nullopt;
		}

		if (ai_state.danger_navigation_request.has_value()) {
			return ai_state.danger_navigation_request;
		}

		if (ai_state.take_cover_navigation_request.has_value()) {
			return ai_state.take_cover_navigation_request;
		}

		return new_request;
	}();

	/*
		Re-initialize navigation when the request changes OR when there is a
		live request but no active path. The latter case happens when a prior
		start_navigating_to() failed (e.g. bot was momentarily on a portal
		cell while a force pushed it around) — without retrying, the cached
		request would equal the next request and the bot would freeze.
	*/
	const bool needs_init =
		effective_request != ai_state.current_navigation_request
		|| (effective_request.has_value() && !ai_state.is_navigating())
	;

	if (needs_init) {
		AI_LOG("Navigation request changed or retrying - reinitializing");

		/*
			Clear first so we never leave a stale (cached request, null path)
			pair behind if start_navigating_to fails below.
		*/
		ai_state.clear_navigation();

		if (effective_request.has_value()) {
			const auto physics_hints = make_physics_path_hints(cosm);
			auto new_navigation = ::start_navigating_to(character_pos, effective_request->target, navmesh, &physics_hints, pathfinding_ctx);

			if (new_navigation.has_value()) {
				new_navigation->exact_destination = effective_request->exact;
				ai_state.navigation = std::move(new_navigation);
				ai_state.current_navigation_request = effective_request;
			}
			else {
				/*
					Don't cache the failed request — leave current_navigation_request
					at nullopt so the next AI tick re-enters this branch and retries.
				*/
				LOG("AI ERROR: start_navigating_to FAILED for bot at (%x,%x) -> target (%x,%x). Waypoint unreachable?",
					character_pos.x, character_pos.y, effective_request->target.pos.x, effective_request->target.pos.y);
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

	movement.flags.walking = ::should_walk_silently(ai_state.last_behavior, bot_faction, ai_state.escaping_explosion, bomb_planted);
	movement.flags.sprinting = ::should_sprint(ai_state.last_behavior, move_result.can_sprint);
	movement.flags.dashing = ::should_dash_for_combat(ai_state.last_behavior, ai_state.combat_target, character_pos);

	if (auto* sentience = character_handle.find<components::sentience>()) {
		auto& consciousness = sentience->get<consciousness_meter_instance>();

		/*
			Sprint cooldown: forbid sprinting when stamina <= 1 until it regenerates up to a threshold.
		*/
		const auto max_consciousness = consciousness.maximum;
		const auto threshold_consciousness = max_consciousness / 2.3f;

		if (ai_state.sprint_cooldown) {
			if (consciousness.value >= threshold_consciousness) {
				ai_state.sprint_cooldown = false;
			}
		}
		else {
			if (consciousness.value <= 1.0f) {
				ai_state.sprint_cooldown = true;
			}
		}

		if (ai_state.sprint_cooldown) {
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
		===========================================================================
		PHASE 8: Emergency avoidance layer (bot avoidance + danger avoidance).

		should_run_avoidance_update throttles computation to a subset of bots
		per frame and is shared between both avoidance functions.
		Danger avoidance is applied last so it has higher priority.
		===========================================================================
	*/

	constexpr std::size_t AVOIDANCE_BOTS_PER_FRAME = 4;

	const bool should_run_avoidance_update = [&]() {
		if (num_bots == 0 || is_freeze_time) {
			return false;
		}

		const auto num_groups = (num_bots + AVOIDANCE_BOTS_PER_FRAME - 1) / AVOIDANCE_BOTS_PER_FRAME;

		if (num_groups <= 1) {
			return true;
		}

		const auto current_group = static_cast<std::size_t>(cosm.get_total_steps_passed()) % num_groups;
		return (bot_index / AVOIDANCE_BOTS_PER_FRAME) == current_group;
	}();

	const bool avoidance_overrode = ::update_bot_avoidance(ctx, movement, should_run_avoidance_update, is_freeze_time, is_thinking_what_to_buy);

	const bool take_cover_overrode = ::update_take_cover(
		ctx,
		movement,
		navmesh,
		move_result,
		should_run_avoidance_update,
		is_freeze_time,
		target_acquired
	);

	const bool fully_stunned = is_blinded && is_deafened;
	const bool can_sense_anything = !fully_stunned;

	const bool is_currently_defusing = [&]() {
		if (const auto* defuse = ::get_behavior_if<ai_behavior_defuse>(ai_state.last_behavior)) {
			return defuse->is_defusing;
		}

		return false;
	}();

	const bool danger_overrode = can_sense_anything && ::update_danger_avoidance(
		ctx,
		movement,
		navmesh,
		move_result,
		dt_secs,
		should_run_avoidance_update,
		is_freeze_time
	);

	/* Bullet avoidance — applied last, overrides all other avoidance layers. */
	const bool bullet_overrode = !is_currently_defusing && can_sense_anything && ::update_bullet_avoidance(
		ctx,
		movement,
		dt_secs,
		difficulty,
		should_run_avoidance_update,
		is_freeze_time,
		is_thinking_what_to_buy
	);

	if (avoidance_overrode || take_cover_overrode || danger_overrode || bullet_overrode) {
		if (auto* patrol = ::get_behavior_if<ai_behavior_patrol>(ai_state.last_behavior)) {
			if (patrol->is_camping()) {
				patrol->camp_timer = 0.0f;
				patrol->twitch_direction = std::nullopt;
			}
		}
	}

	/*
		Calculate and apply hand_flags (triggers, planting).
	*/
	{
		const auto hand_flags = ::calc_hand_flags(
			ai_state.last_behavior,
			target_acquired,
			aim_pos,
			character_handle,
			ai_state.melee_reaction_timer,
			::get_melee_reaction_time_secs(difficulty),
			dt_secs
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
		ai_state.clear_navigation();
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
		Skip if deafened by a flashbang.
	*/
	const auto* const bot_sentience_ps = character_handle.find<components::sentience>();
	const bool is_deafened_ps = bot_sentience_ps != nullptr && bot_sentience_ps->audio_flash_secs > 2.0f;

	if (!is_deafened_ps) {
		::listen_for_sound_cues(ctx, step, is_ffa, global_time_secs, bomb_planted);
	}

	/*
		Check for teleportation messages - clear pathfinding if this bot was teleported.
	*/
	const auto& game_notifications = step.get_queue<messages::game_notification>();

	for (const auto& notification : game_notifications) {
		if (const auto* tp = std::get_if<messages::teleportation>(&notification.payload)) {
			if (tp->teleported == controlled_character_id) {
				/*
					Bot was teleported - drop the stale path so the next AI
					tick re-initializes navigation from the bot's new position.
				*/
				ai_state.clear_navigation();
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
					const auto hearing_dist = gun_def.muzzle_shot_sound.modifier.max_distance * 0.8f;
					const auto dist_sq = (character_pos - muzzle_pos).length_sq();

					if (dist_sq > hearing_dist * hearing_dist) {
						out_of_range = true;
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

		const auto bot_can_penetrate_to_shooter = ::can_weapon_penetrate(character_handle, muzzle_pos);

		if (!raycast.hit || ::can_weapon_penetrate(shooter, character_pos) || bot_can_penetrate_to_shooter) {
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

real32 get_melee_reaction_time_secs(const difficulty_type difficulty) {
	switch (difficulty) {
		case difficulty_type::EASY:   return 0.22f;
		case difficulty_type::MEDIUM: return 0.15f;
		case difficulty_type::HARD:   return 0.00f;
		default:                      return 0.05f;
	}
}
