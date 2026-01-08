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
#include "game/detail/inventory/weapon_reloading.hpp"
#include "game/detail/pathfinding.h"

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

arena_ai_result update_arena_mode_ai(
	const cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	const entity_id controlled_character_id,
	const money_type money,
	const bool is_ffa,
	xorshift_state& stable_round_rng,
	const difficulty_type difficulty
) {
	auto stable_rng = randomization(stable_round_rng);

	auto scope = augs::scope_guard([&]() {
		stable_round_rng = stable_rng.generator;
	});

	const entity_handle character_handle = cosm[controlled_character_id];

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

	/*
		High-level AI flow.
	*/

	/*
		Listen for footsteps before checking for visible enemies.
	*/
	::listen_for_footsteps(ctx, step, is_ffa);

	::update_random_movement_target(ctx, dt_secs);

	const auto closest_enemy = ::find_closest_enemy(ctx, is_ffa);
	const bool sees_target = closest_enemy.is_set();
	const bool should_react = ::update_alertness(ai_state, sees_target, dt_secs, difficulty);
	const bool has_target = sees_target && should_react;

	if (sees_target) {
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

	movement.flags.sprinting = false;
	movement.flags.dashing = false;

	const auto actual_movement_direction = [&]() {
		if (!pause_chase && ai_state.last_seen_target.is_set()) {
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
	const cosmos& cosm,
	const logic_step step,
	arena_mode_ai_state& ai_state,
	const entity_id controlled_character_id,
	const bool is_ffa
) {
	const auto character_handle = cosm[controlled_character_id];

	if (!character_handle.alive()) {
		return;
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
