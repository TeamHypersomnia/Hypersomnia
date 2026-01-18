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
#include "game/modes/ai/tasks/update_alertness.hpp"
#include "game/modes/ai/tasks/handle_aiming_and_trigger.hpp"
#include "game/modes/ai/tasks/interpolate_crosshair.hpp"
#include "game/modes/ai/tasks/handle_purchases.hpp"
#include "game/modes/ai/tasks/listen_for_footsteps.hpp"
#include "game/modes/ai/tasks/ai_behavior_tree.hpp"
#include "game/modes/ai/tasks/ai_waypoint_helpers.hpp"
#include "game/modes/ai/behaviors/eval_behavior_tree.hpp"
#include "game/modes/ai/behaviors/ai_behavior_process_ctx.hpp"
#include "game/modes/ai/behaviors/ai_behavior_patrol_process.hpp"
#include "game/modes/ai/behaviors/ai_behavior_defuse_process.hpp"
#include "game/modes/ai/intents/calc_pathfinding_request.hpp"
#include "game/modes/ai/intents/calc_movement_direction.hpp"
#include "game/modes/ai/intents/calc_wielding_intent.hpp"
#include "game/modes/ai/intents/calc_assigned_waypoint.hpp"
#include "game/modes/ai/intents/should_helpers.hpp"

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
		PHASE 1: Update combat target tracking (before behavior tree evaluation).
		===========================================================================
	*/

	/* Listen for footsteps and update combat target from sound cues. */
	::listen_for_footsteps(ctx, step, is_ffa, global_time_secs);

	/* Check for visible enemies and update combat_target. */
	const auto closest_enemy = ::find_closest_enemy(ctx, is_ffa);
	const bool sees_target = closest_enemy.is_set();
	const bool should_react = ::update_alertness(ai_state, sees_target, dt_secs, difficulty);
	const bool has_target = sees_target && should_react;

	if (has_target) {
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
		PHASE 3: Calculate pathfinding request FIRST (before process()).
		We need path_completed to pass to process().
		===========================================================================
	*/

	const auto new_request = ::calc_current_pathfinding_request(
		cosm,
		ai_state.last_behavior,
		ai_state.combat_target,
		team_state,
		bot_player_id,
		bot_faction,
		character_pos,
		bomb_planted,
		bomb_entity,
		global_time_secs
	);

	AI_LOG_NVPS(new_request.has_request, new_request.exact, new_request.is_bomb_target);

	/* Check if request changed - reinitialize pathfinding. */
	if (new_request != ai_state.current_pathfinding_request) {
		AI_LOG("Pathfinding request changed - reinitializing");
		ai_state.current_pathfinding_request = new_request;
		ai_state.clear_pathfinding();

		if (new_request.has_request) {
			::start_pathfinding_to(ai_state, character_pos, new_request.target, navmesh, nullptr);

			if (ai_state.is_pathfinding_active()) {
				ai_state.pathfinding->exact_destination = new_request.exact;
			}
		}
	}

	/*
		===========================================================================
		PHASE 4: Calculate movement direction (stateless, uses behavior variant).
		===========================================================================
	*/

	const auto move_result = ::calc_current_movement_direction(
		ai_state.last_behavior,
		ai_state.pathfinding,
		character_pos,
		navmesh,
		character_handle,
		dt_secs
	);

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

	/* Update crosshair offset from movement calculation. */
	if (move_result.crosshair_offset != vec2::zero) {
		ai_state.target_crosshair_offset = move_result.crosshair_offset;
	}

	/*
		===========================================================================
		PHASE 6: Calculate and apply weapon wielding (via intent calculator).
		===========================================================================
	*/

	const auto wielding_intent = ::calc_wielding_intent(ai_state.last_behavior, character_handle);

	if (wielding_intent.should_change) {
		::perform_wielding(step, character_handle, wielding_intent.desired_wielding);
	}

	/*
		===========================================================================
		PHASE 7: Apply movement, aiming, crosshair, purchases.
		===========================================================================
	*/

	movement.flags.walking = ::should_walk_silently(ai_state.last_behavior);
	movement.flags.sprinting = ::should_sprint(ai_state.last_behavior, move_result.can_sprint);
	movement.flags.dashing = ::should_dash_for_combat(ai_state.last_behavior, ai_state.combat_target, character_pos);

	if (move_result.direction.has_value()) {
		movement.flags.set_from_closest_direction(*move_result.direction);
	}
	else {
		movement.flags.set_from_closest_direction(vec2::zero);
	}

	AI_LOG_NVPS(movement.flags.walking, movement.flags.sprinting, movement.flags.dashing);

	::handle_aiming_and_trigger(ctx, has_target, closest_enemy);
	::interpolate_crosshair(ctx, has_target, dt_secs, difficulty, move_result.is_navigating);

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
	const auto global_time_secs = cosm.get_total_seconds_passed();
	const auto& physics = cosm.get_solvable_inferred().physics;
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
			Use force_acquire since damage is always significant.
		*/
		ai_state.combat_target.force_acquire(
			rng,
			global_time_secs,
			attacker_id,
			attacker_pos
		);
	}

	if (auto* movement = character_handle.find<components::movement>()) {
		movement->flags.sprinting = false;
		movement->flags.dashing = false;
	}
}
