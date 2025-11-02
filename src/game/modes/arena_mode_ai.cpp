#include "game/components/movement_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/gun_component.h"
#include "game/components/sentience_component.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/cosmos/entity_handle.h"
#include "augs/math/math.h"
#include "augs/misc/randomization.h"
#include "game/modes/arena_mode_ai.h"
#include "game/enums/filters.h"
#include "game/cosmos/for_each_entity.h"
#include "game/modes/arena_mode_ai_structs.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/modes/detail/item_purchase_logic.hpp"
#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "augs/misc/scope_guard.h"
#include "game/detail/sentience/sentience_getters.h"
#include "game/debug_drawing_settings.h"
#include "game/messages/gunshot_message.h"
#include "game/detail/inventory/weapon_reloading.hpp"

struct ai_character_context {
	arena_mode_ai_state& ai_state;
	const vec2 character_pos;
	const physics_world_cache& physics;
	const cosmos& cosm;
	const entity_handle character_handle;
};

static void update_random_movement_target(
	const ai_character_context& ctx,
	const float dt_secs
) {
	ctx.ai_state.movement_timer_remaining -= dt_secs;

	if (ctx.ai_state.movement_timer_remaining <= 0.0f) {
		const auto seed = ctx.cosm.get_rng_seed_for(ctx.character_handle);
		auto rng = randomization(seed);

		const auto random_direction = [&]() {
			if (ctx.ai_state.chase_timeout > 0.0f) {
				const auto rot = rng.randval(-135, 135);
				return (ctx.character_pos - ctx.ai_state.last_target_position).normalize().rotate(rot);
			}

			return rng.random_point_on_unit_circle<real32>();
		}();

		const auto raycast_distance = 1500.0f;
		const auto raycast_end = ctx.character_pos + random_direction * raycast_distance;
		const auto filter = predefined_queries::pathfinding();

		const auto raycast = ctx.physics.ray_cast_px(
			ctx.cosm.get_si(),
			ctx.character_pos,
			raycast_end,
			filter,
			ctx.character_handle
		);

		ctx.ai_state.random_movement_target = raycast.hit ? raycast.intersection : raycast_end;
		ctx.ai_state.movement_duration_secs = rng.randval(1.0f, 3.0f);
		ctx.ai_state.movement_timer_remaining = ctx.ai_state.movement_duration_secs;
	}
}

static entity_id find_closest_enemy(
	const ai_character_context& ctx,
	const bool is_ffa
) {
	entity_id closest_enemy = entity_id::dead();
	float closest_distance = std::numeric_limits<float>::max();
	const auto filter = predefined_queries::line_of_sight();

	const auto current_aim_direction = [&]() {
		if (auto crosshair = ctx.character_handle.find_crosshair()) {
			return vec2(crosshair->base_offset).normalize();
		}

		return vec2::zero;
	}();

	ctx.cosm.for_each_having<components::sentience>(
		[&](const auto& entity) {
			if (entity == ctx.character_handle || !entity.alive() || !::sentient_and_conscious(entity)) {
				return;
			}

			if (!is_ffa) {
				const auto bot_faction = ctx.character_handle.get_official_faction();
				const auto target_faction = entity.get_official_faction();

				if (bot_faction == target_faction) {
					return;
				}
			}

			const auto offset = entity.get_logic_transform().pos - ctx.character_pos;
			const auto distance = offset.length();

			if (distance < closest_distance && repro::fabs(offset.y) < 1080.0f && repro::fabs(offset.x) < 1920.0f) {
				const auto direction_to_enemy = vec2(offset).normalize();
				const auto angle_to_enemy = current_aim_direction.degrees_between(direction_to_enemy);

				if (angle_to_enemy <= 70.0f) {
					const auto raycast = ctx.physics.ray_cast_px(
						ctx.cosm.get_si(),
						ctx.character_pos,
						entity.get_logic_transform().pos,
						filter,
						ctx.character_handle
					);

					if (!raycast.hit) {
						closest_enemy = entity;
						closest_distance = distance;
					}
				}
			}
		}
	);

	return closest_enemy;
}

static bool update_alertness(
	arena_mode_ai_state& ai_state,
	const bool sees_target,
	const float dt_secs,
	const difficulty_type difficulty
) {
	if (sees_target) {
		ai_state.alertness_time += dt_secs;
	}
	else {
		ai_state.alertness_time -= dt_secs;
		ai_state.alertness_time = std::max(0.0f, ai_state.alertness_time);
	}

	const auto reaction_threshold_secs = [&]() {
		switch (difficulty) {
			case difficulty_type::HARD: return 0.25f;
			case difficulty_type::MEDIUM: return 0.8f;
			case difficulty_type::EASY: return 1.5f;
			default: return 1.0f;
		}
	}();

	ai_state.alertness_time = std::min(reaction_threshold_secs + 0.2f, ai_state.alertness_time);

	return ai_state.alertness_time >= reaction_threshold_secs;
}

static void update_target_tracking(
	const ai_character_context& ctx,
	const entity_id closest_enemy
) {
	const auto target_pos = ctx.cosm[closest_enemy].get_logic_transform().pos;
	ctx.ai_state.last_seen_target = closest_enemy;
	ctx.ai_state.chase_remaining_time = 5.0f;
	ctx.ai_state.last_target_position = target_pos;
	ctx.ai_state.has_dashed_for_last_seen_target = false;
}

template <class SetMovementTarget>
static vec2 handle_active_chase(
	const ai_character_context& ctx,
	const bool sees_target,
	const float dt_secs,
	SetMovementTarget set_movement_target
) {
	const auto distance_to_last_seen = (ctx.ai_state.last_target_position - ctx.character_pos).length();
	const auto reached_threshold = 20.0f;

	ctx.ai_state.chase_remaining_time -= dt_secs;

	if (distance_to_last_seen < reached_threshold && !sees_target) {
		ctx.ai_state.chase_remaining_time = 0.0f;
	}

	if (ctx.ai_state.chase_remaining_time > 0.0f) {
		if (sees_target) {
			if (distance_to_last_seen < 250.0f) {
				if (ctx.ai_state.chase_timeout < 0.0f) {
					ctx.ai_state.movement_timer_remaining = 0.0f;
				}

				ctx.ai_state.chase_timeout = 1.0f;
				const auto dir = set_movement_target(ctx.ai_state.random_movement_target, false, false);

				if (DEBUG_DRAWING.draw_ai_info) {
					DEBUG_LOGIC_STEP_LINES.emplace_back(yellow, ctx.character_pos, ctx.ai_state.last_target_position);
				}

				return dir;
			}
			else {
				const auto dir = set_movement_target(ctx.ai_state.last_target_position, false, false);

				if (DEBUG_DRAWING.draw_ai_info) {
					DEBUG_LOGIC_STEP_LINES.emplace_back(yellow, ctx.character_pos, ctx.ai_state.last_target_position);
				}

				return dir;
			}
		}
		else {
			const auto should_dash = distance_to_last_seen < 400.0f && !ctx.ai_state.has_dashed_for_last_seen_target;

			if (should_dash) {
				ctx.ai_state.has_dashed_for_last_seen_target = true;
			}

			const auto dir = set_movement_target(ctx.ai_state.last_target_position, true, should_dash);

			if (DEBUG_DRAWING.draw_ai_info) {
				DEBUG_LOGIC_STEP_LINES.emplace_back(orange, ctx.character_pos, ctx.ai_state.last_target_position);
			}

			return dir;
		}
	}
	else {
		ctx.ai_state.last_seen_target = entity_id::dead();
		ctx.ai_state.chase_remaining_time = 0.0f;
		return set_movement_target(ctx.ai_state.random_movement_target, false, false);
	}
}

template <class SetMovementTarget>
static vec2 handle_random_movement(
	const ai_character_context& ctx,
	const bool pause_chase,
	SetMovementTarget set_movement_target
) {
	const auto distance_to_target = (ctx.ai_state.random_movement_target - ctx.character_pos).length();

	if (DEBUG_DRAWING.draw_ai_info) {
		DEBUG_LOGIC_STEP_LINES.emplace_back(pause_chase ? cyan : white, ctx.character_pos, ctx.ai_state.random_movement_target);
	}

	if (distance_to_target < 140.0f) {
		ctx.ai_state.movement_timer_remaining = 0.0f;
	}

	return set_movement_target(ctx.ai_state.random_movement_target, false, false);
}

static void handle_aiming_and_trigger(
	const ai_character_context& ctx,
	const bool has_target,
	const entity_id closest_enemy
) {
	bool trigger = false;

	if (has_target) {
		const auto target_pos = ctx.cosm[closest_enemy].get_logic_transform().pos;
		const auto aim_direction = target_pos - ctx.character_pos;
		ctx.ai_state.target_crosshair_offset = aim_direction;

		if (auto crosshair = ctx.character_handle.find_crosshair()) {
			const auto current_aim = vec2(crosshair->base_offset).normalize();
			const auto target_aim = vec2(aim_direction).normalize();
			const auto angle_diff = current_aim.degrees_between(target_aim);

			trigger = angle_diff <= 25.0f;
		}
	}

	if (ctx.character_handle.is_frozen()) {
		trigger = false;
	}

	if (auto sentience = ctx.character_handle.find<components::sentience>()) {
		sentience->hand_flags[0] = sentience->hand_flags[1] = trigger;
	}
}

static void interpolate_crosshair(
	const ai_character_context& ctx,
	const bool has_target,
	const float dt_secs,
	const difficulty_type difficulty
) {
	if (auto crosshair = ctx.character_handle.find_crosshair()) {
		const auto average_factor = 0.5f;
		auto averages_per_sec = has_target ? 4.0f : 3.0f;

		if (difficulty == difficulty_type::EASY) {
			averages_per_sec = 1.5f;
		}

		const auto averaging_constant = 1.0f - static_cast<real32>(repro::pow(average_factor, averages_per_sec * dt_secs));

		crosshair->base_offset = augs::interp(crosshair->base_offset, ctx.ai_state.target_crosshair_offset, averaging_constant);
	}
}

static std::optional<item_flavour_id> handle_purchases(
	const ai_character_context& ctx,
	const money_type money,
	const float dt_secs,
	randomization& stable_rng
) {
	if (ctx.ai_state.already_tried_to_buy || money <= 0) {
		return std::nullopt;
	}

	if (ctx.ai_state.purchase_decision_countdown < 0.0f) {
		ctx.ai_state.purchase_decision_countdown = stable_rng.randval(1.0f, 3.0f);
	}

	ctx.ai_state.purchase_decision_countdown -= dt_secs;

	if (ctx.ai_state.purchase_decision_countdown > 0.0f) {
		return std::nullopt;
	}

	ctx.ai_state.already_tried_to_buy = true;

	auto get_owned_guns = [&]() {
		bool has_only_pistols = true;
		int weapon_count = 0;
		std::vector<item_flavour_id> owned_guns;

		ctx.character_handle.for_each_contained_item_recursive(
			[&](const auto& item) {
				if (const auto gun = item.template find<invariants::gun>()) {
					weapon_count++;
					owned_guns.push_back(item.get_flavour_id());

					if (gun->buy_type != buy_menu_type::PISTOLS) {
						has_only_pistols = false;
					}
				}
			}
		);

		return std::tuple{ weapon_count, has_only_pistols, owned_guns };
	};

	auto try_buy_gun = [&](const std::vector<item_flavour_id>& owned_guns) -> std::optional<item_flavour_id> {
		std::vector<item_flavour_id> affordable_pistols;
		std::vector<item_flavour_id> affordable_non_pistols;

		ctx.cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
			if (!factions_compatible(ctx.character_handle, id)) {
				return;
			}

			if (flavour.template get<invariants::gun>().bots_ban) {
				return;
			}

			const auto price = *::find_price_of(ctx.cosm, item_flavour_id(id));

			if (price <= money) {
				const auto buy_type = flavour.template get<invariants::gun>().buy_type;

				if (buy_type == buy_menu_type::PISTOLS) {
					affordable_pistols.push_back(item_flavour_id(id));
				}
				else {
					affordable_non_pistols.push_back(item_flavour_id(id));
				}
			}
		});

		const auto& weapons_to_choose_from =
			!affordable_non_pistols.empty() ? affordable_non_pistols : affordable_pistols;

		if (weapons_to_choose_from.empty()) {
			return std::nullopt;
		}

		const auto random_index = stable_rng.randval(0u, static_cast<unsigned>(weapons_to_choose_from.size() - 1));
		const auto bought = weapons_to_choose_from[random_index];

		if (found_in(owned_guns, bought)) {
			return std::nullopt;
		}

		return bought;
	};

	auto try_buy_armor = [&]() -> std::optional<item_flavour_id> {
		std::vector<item_flavour_id> affordable_armor;

		ctx.cosm.for_each_flavour_having<invariants::tool>([&](const auto& id, const auto& flavour) {
			if (::is_armor_like(flavour)) {
				const auto price = *::find_price_of(ctx.cosm, item_flavour_id(id));

				if (price <= money) {
					affordable_armor.push_back(item_flavour_id(id));
				}
			}
		});

		if (affordable_armor.empty()) {
			return std::nullopt;
		}

		const auto random_index = stable_rng.randval(0u, static_cast<unsigned>(affordable_armor.size() - 1));
		return affordable_armor[random_index];
	};

	const auto [weapon_count, has_only_pistols, owned_guns] = get_owned_guns();

	if (weapon_count <= 1 || has_only_pistols) {
		if (const auto gun = try_buy_gun(owned_guns)) {
			return gun;
		}
	}

	return try_buy_armor();
}

arena_ai_result update_arena_mode_ai(
	const arena_mode::input& in,
	const logic_step step,
	arena_mode_player& player,
	const bool is_ffa,
	xorshift_state& stable_round_rng,
	const difficulty_type difficulty
) {
	auto stable_rng = randomization(stable_round_rng);

	auto scope = augs::scope_guard([&]() {
		stable_round_rng = stable_rng.generator;
	});

	const auto money = player.stats.money;
	auto& cosm = in.cosm;
	const entity_handle character_handle = cosm[player.controlled_character_id];

	if (!character_handle.alive()) {
		return arena_ai_result{};
	}

	auto& movement = character_handle.get<components::movement>();
	const auto character_pos = character_handle.get_logic_transform().pos;
	const auto dt_secs = step.get_delta().in_seconds();
	const auto& physics = cosm.get_solvable_inferred().physics;

	const auto ctx = ai_character_context{
		player.ai_state,
		character_pos,
		physics,
		cosm,
		character_handle
	};

	auto set_movement_target = [&](const vec2 target_pos, const bool sprint, const bool dash) {
		auto offset = target_pos - character_pos;
		player.ai_state.target_crosshair_offset = offset;
		movement.flags.sprinting = sprint;
		movement.flags.dashing = dash;
		return offset.normalize();
	};

	/*
		High-level AI flow.
	*/

	update_random_movement_target(ctx, dt_secs);

	const auto closest_enemy = find_closest_enemy(ctx, is_ffa);
	const bool sees_target = closest_enemy.is_set();
	const bool should_react = update_alertness(player.ai_state, sees_target, dt_secs, difficulty);
	const bool has_target = sees_target && should_react;

	if (sees_target) {
		update_target_tracking(ctx, closest_enemy);
		player.ai_state.has_dashed_for_last_seen_target = false;
	}

	const bool reloading = ::is_currently_reloading(character_handle);

	if (player.ai_state.chase_timeout > 0.0f) {
		player.ai_state.chase_timeout -= dt_secs;
	}

	const bool pause_chase = player.ai_state.chase_timeout > 0.0f || reloading;

	if (pause_chase) {
		player.ai_state.chase_remaining_time -= dt_secs;
	}

	movement.flags.sprinting = false;
	movement.flags.dashing = false;

	const auto actual_movement_direction = [&]() {
		if (!pause_chase && player.ai_state.last_seen_target.is_set()) {
			return handle_active_chase(ctx, sees_target, dt_secs, set_movement_target);
		}
		else {
			return handle_random_movement(ctx, pause_chase, set_movement_target);
		}
	}();

	movement.flags.set_from_closest_direction(actual_movement_direction);

	handle_aiming_and_trigger(ctx, has_target, closest_enemy);
	interpolate_crosshair(ctx, has_target, dt_secs, difficulty);

	arena_ai_result result;
	result.item_purchase = handle_purchases(ctx, money, dt_secs, stable_rng);

	return result;
} 

void post_solve_arena_mode_ai(const arena_mode::input& in, arena_mode_player& player, const logic_step step) {
	if (!player.controlled_character_id.is_set()) {
		return;
	}

	auto& cosm = in.cosm;
	const auto character_handle = cosm[player.controlled_character_id];

	if (!character_handle.alive()) {
		return;
	}

	const auto& gunshots = step.get_queue<messages::gunshot_message>();
	const auto bot_faction = character_handle.get_official_faction();
	const auto is_ffa = in.rules.is_ffa();
	const auto character_pos = character_handle.get_logic_transform().pos;
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto filter = predefined_queries::line_of_sight();

	auto current_target_distance = [&]() {
		if (player.ai_state.last_seen_target.is_set()) {
			return (player.ai_state.last_target_position - character_pos).length();
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
		player.ai_state.last_seen_target = shooter_id;
		player.ai_state.chase_remaining_time = 5.0f;
		player.ai_state.last_target_position = muzzle_pos;
		player.ai_state.has_dashed_for_last_seen_target = false;
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

			if (!player.ai_state.last_seen_target.is_set() || dist < current_target_distance) {
				update_target_from_gunshot(shot.capability, muzzle_pos, dist);
			}
		}
	}

	if (auto* movement = character_handle.find<components::movement>()) {
		movement->flags.sprinting = false;
		movement->flags.dashing = false;
	}
} 