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

	// Track the actual movement direction
	vec2 actual_movement_direction = vec2::zero;

	player.ai_state.movement_timer_remaining -= dt_secs;

	if (player.ai_state.movement_timer_remaining <= 0.0f) {
		// Raycast in a random direction to find a valid movement target
		const auto seed = cosm.get_rng_seed_for(player.controlled_character_id);
		auto rng = randomization(seed);
		
		const auto random_direction = rng.random_point_in_unit_circle<real32>();
		const auto raycast_distance = 1500.0f; // Reasonable distance for movement target
		const auto raycast_end = character_pos + random_direction * raycast_distance;
		
		const auto& physics = cosm.get_solvable_inferred().physics;
		const auto filter = predefined_queries::pathfinding();
		
		const auto raycast = physics.ray_cast_px(
			cosm.get_si(),
			character_pos,
			raycast_end,
			filter,
			character_handle
		);
		
		if (raycast.hit) {
			// Use the intersection point as movement target
			player.ai_state.random_movement_target = raycast.intersection;
		}
		else {
			// If no hit, use the random direction
			player.ai_state.random_movement_target = raycast_end;
		}
		
		player.ai_state.movement_duration_secs = rng.randval(1.0f, 3.0f);
		player.ai_state.movement_timer_remaining = player.ai_state.movement_duration_secs;
	}

	// Simple target detection - look for enemies in the same faction
	entity_id closest_enemy = entity_id::dead();
	float closest_distance = std::numeric_limits<float>::max();

	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto filter = predefined_queries::line_of_sight();

	// Get current aim direction from actual crosshair for line of sight
	vec2 current_aim_direction = vec2::zero;
	if (auto crosshair = character_handle.find_crosshair()) {
		current_aim_direction = vec2(crosshair->base_offset).normalize();
	}
	else {
		// Fallback to movement direction if no crosshair
		current_aim_direction = actual_movement_direction;
	}

	cosm.for_each_having<components::sentience>(
		[&](const auto& entity) {
			if (entity == character_handle) {
				return;
			}

			if (!entity.alive()) {
				return;
			}

			if (!is_ffa) {
				const auto bot_faction = character_handle.get_official_faction();
				const auto target_faction = entity.get_official_faction();
				
				if (bot_faction == target_faction) {
					return;
				}
			}

			if (!::sentient_and_conscious(entity)) {
				return;
			}

			const auto offset = entity.get_logic_transform().pos - character_pos;
			const auto distance = (offset).length();
			
			if (distance < closest_distance && repro::fabs(offset.y) < 1080.0f && repro::fabs(offset.x) < 1920.0f) {
				const auto direction_to_enemy = (entity.get_logic_transform().pos - character_pos).normalize();
				const auto angle_to_enemy = current_aim_direction.degrees_between(direction_to_enemy);
				
				if (angle_to_enemy <= 70.0f) {
					const auto line_from = character_pos;
					const auto line_to = entity.get_logic_transform().pos;
					
					const auto raycast = physics.ray_cast_px(
						cosm.get_si(),
						line_from,
						line_to,
						filter,
						character_handle
					);

					if (!raycast.hit) {
						closest_enemy = entity;
						closest_distance = distance;
					}
				}
			}
		}
	);

	const bool sees_target = closest_enemy.is_set();

	if (sees_target) {
		player.ai_state.alertness_time += dt_secs;
	}
	else {
		player.ai_state.alertness_time -= dt_secs;
		player.ai_state.alertness_time = std::max(0.0f, player.ai_state.alertness_time);
	}

	const auto reaction_threshold_secs = [&]() {
		switch (difficulty) {
			case difficulty_type::HARD: return 0.25f;
			case difficulty_type::MEDIUM: return 0.8f;
			case difficulty_type::EASY: return 1.5f;
			default: return 1.0f;
		}
	}();
	
	const bool should_react = player.ai_state.alertness_time >= reaction_threshold_secs;
	const bool has_target = sees_target && should_react;

	if (sees_target) {
		const auto target_pos = cosm[closest_enemy].get_logic_transform().pos;
		player.ai_state.last_seen_target = closest_enemy;
		player.ai_state.chase_remaining_time = 5.0f; // 5 second chase timeout
		player.ai_state.last_target_position = target_pos;
		player.ai_state.has_dashed_for_last_seen_target = false; // Reset dash flag for new target
	}
	
	player.ai_state.alertness_time = std::min(reaction_threshold_secs + 0.2f, player.ai_state.alertness_time);

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

	if (!pause_chase && player.ai_state.last_seen_target.is_set()) {
		const auto distance_to_last_seen = (player.ai_state.last_target_position - character_pos).length();
		const auto reached_threshold = 20.0f;
		
		// Update chase timer using delta time
		player.ai_state.chase_remaining_time -= dt_secs;
		
		if (distance_to_last_seen < reached_threshold && !has_target) {
			player.ai_state.chase_remaining_time = 0.0f;
		}

		if (player.ai_state.chase_remaining_time > 0.0f) {
			if (has_target) {
				if (distance_to_last_seen > 200.0f) {
					actual_movement_direction = (player.ai_state.last_target_position - character_pos).normalize();
					player.ai_state.target_crosshair_offset = player.ai_state.last_target_position - character_pos;

					if (DEBUG_DRAWING.draw_ai_info) {
						DEBUG_LOGIC_STEP_LINES.emplace_back(yellow, character_pos, player.ai_state.last_target_position);
					}
				}
				else {
					player.ai_state.chase_timeout = 3.0f;
					actual_movement_direction = (player.ai_state.random_movement_target - character_pos).normalize();
					player.ai_state.target_crosshair_offset = player.ai_state.random_movement_target - character_pos;
				}
			}
			else {
				actual_movement_direction = (player.ai_state.last_target_position - character_pos).normalize();
				player.ai_state.target_crosshair_offset = player.ai_state.last_target_position - character_pos;

				movement.flags.sprinting = true;

				const auto distance_to_last_seen = (player.ai_state.last_target_position - character_pos).length();
				if (distance_to_last_seen < 400.0f && !player.ai_state.has_dashed_for_last_seen_target) {
					movement.flags.dashing = true;
					player.ai_state.has_dashed_for_last_seen_target = true;
				}

				if (DEBUG_DRAWING.draw_ai_info) {
					DEBUG_LOGIC_STEP_LINES.emplace_back(orange, character_pos, player.ai_state.last_target_position);
				}
			}
		}
		else {
			player.ai_state.last_seen_target = entity_id::dead();
			player.ai_state.chase_remaining_time = 0.0f;

			// Resume random movement - calculate direction dynamically
			actual_movement_direction = (player.ai_state.random_movement_target - character_pos).normalize();
			player.ai_state.target_crosshair_offset = player.ai_state.random_movement_target - character_pos;
		}
	}
	else {
		player.ai_state.target_crosshair_offset = player.ai_state.random_movement_target - character_pos;
		// Random movement when no target - calculate direction dynamically
		actual_movement_direction = (player.ai_state.random_movement_target - character_pos).normalize();
		
		// Check if we've reached the current movement target (within 100 units)
		const auto distance_to_target = (player.ai_state.random_movement_target - character_pos).length();

		if (DEBUG_DRAWING.draw_ai_info) {
			DEBUG_LOGIC_STEP_LINES.emplace_back(pause_chase ? cyan : white, character_pos, player.ai_state.random_movement_target);
		}

		if (distance_to_target < 140.0f) {
			// Force a new movement direction by resetting the timer
			player.ai_state.movement_timer_remaining = 0.0f;
		}
	}

	if (has_target) {
		// If target is in POV, reset dash flag
		player.ai_state.has_dashed_for_last_seen_target = false;
	}

	// Set movement direction flags at the very end, but before custom sprint/dash logic
	movement.flags.set_from_closest_direction(actual_movement_direction);

	const bool target_in_range = has_target;

	bool trigger = false;

	// Set target crosshair offset based on current situation
	if (has_target) {
		const auto target_pos = cosm[closest_enemy].get_logic_transform().pos;
		auto aim_direction = (target_pos - character_pos);
		player.ai_state.target_crosshair_offset = aim_direction;

		// Check if crosshair is aimed within 10 degrees of target
		if (auto crosshair = character_handle.find_crosshair()) {
			const auto current_aim = vec2(crosshair->base_offset).normalize();
			const auto target_aim = vec2(aim_direction).normalize();
			const auto angle_diff = current_aim.degrees_between(target_aim);
			
			trigger = (angle_diff <= 25.0f) && target_in_range;
		}
	}

	if (character_handle.is_frozen()) {
		trigger = false;
	}

	if (auto sentience = character_handle.find<components::sentience>()) {
		sentience->hand_flags[0] = sentience->hand_flags[1] = trigger;
	}

	// Smoothly interpolate crosshair towards target offset
	if (auto crosshair = character_handle.find_crosshair()) {
		const float average_factor = 0.5f;
		float averages_per_sec = has_target ? 4.0f : 3.0f;

		if (difficulty == difficulty_type::EASY) {
			averages_per_sec = 2.0f;
		}

		const float averaging_constant = 1.0f - static_cast<real32>(repro::pow(average_factor, averages_per_sec * dt_secs));
		
		crosshair->base_offset = augs::interp(crosshair->base_offset, player.ai_state.target_crosshair_offset, averaging_constant);
	}

	// Purchase logic
	arena_ai_result result;
	
	if (!player.ai_state.already_tried_to_buy && money > 0) {
		if (player.ai_state.purchase_decision_countdown < 0.0f) {
			player.ai_state.purchase_decision_countdown = stable_rng.randval(1.0f, 3.0f);
		}

		player.ai_state.purchase_decision_countdown -= dt_secs;

		if (player.ai_state.purchase_decision_countdown <= 0.0f) {
			player.ai_state.already_tried_to_buy = true;

			// Check wielded items to see if we have only one pistol
			bool has_only_pistols = true;

			int weapon_count = 0;
			std::vector<item_flavour_id> owned_guns;
			
			character_handle.for_each_contained_item_recursive(
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

			// Try to buy a gun if we have only one weapon or if we have only one pistol
			if (weapon_count <= 1 || has_only_pistols) {
				std::vector<item_flavour_id> affordable_pistols;
				std::vector<item_flavour_id> affordable_non_pistols;
				
				cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto& flavour) {
					if (!factions_compatible(character_handle, id)) {
						return;
					}

					if (flavour.get<invariants::gun>().bots_ban) {
						return;
					}

					const auto price = *::find_price_of(cosm, item_flavour_id(id));

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

				std::vector<item_flavour_id>* weapons_to_choose_from = nullptr;
				
				if (!affordable_non_pistols.empty()) {
					weapons_to_choose_from = &affordable_non_pistols;
				}
				else if (!affordable_pistols.empty()) {
					weapons_to_choose_from = &affordable_pistols;
				}

				if (weapons_to_choose_from != nullptr && !weapons_to_choose_from->empty()) {
					const auto random_index = stable_rng.randval(0u, static_cast<unsigned>(weapons_to_choose_from->size() - 1));
					const auto bought = (*weapons_to_choose_from)[random_index];

					if (!found_in(owned_guns, bought)) {
						result.item_purchase = bought;
					}
				}
			}

			if (result.item_purchase == std::nullopt) {
				std::vector<item_flavour_id> affordable_armor;
				
				cosm.for_each_flavour_having<invariants::tool>([&](const auto& id, const auto& flavour) {
					if (::is_armor_like(flavour)) {
						const auto price = *::find_price_of(cosm, item_flavour_id(id));
						if (price <= money) {
							affordable_armor.push_back(item_flavour_id(id));
						}
					}
				});

				if (!affordable_armor.empty()) {
					// Pick a random affordable armor
					const auto random_index = stable_rng.randval(0u, static_cast<unsigned>(affordable_armor.size() - 1));
					result.item_purchase = affordable_armor[random_index];
				}
			}
		}
	}

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

	// --- GUNSHOT REACTION LOGIC ---
	const auto& gunshots = step.get_queue<messages::gunshot_message>();
	const auto bot_faction = character_handle.get_official_faction();
	const bool is_ffa = in.rules.is_ffa();
	const auto character_pos = character_handle.get_logic_transform().pos;
	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto filter = predefined_queries::line_of_sight();

	float current_target_distance = std::numeric_limits<float>::max();

	if (player.ai_state.last_seen_target.is_set()) {
		current_target_distance = (player.ai_state.last_target_position - character_pos).length();
	}

	for (const auto& shot : gunshots) {
		if (!shot.capability.is_set()) continue;
		const auto shooter = cosm[shot.capability];
		if (!shooter.alive()) continue;
		if (shooter == character_handle) continue;

		const auto shooter_faction = shooter.get_official_faction();
		bool is_enemy = false;

		if (is_ffa) {
			is_enemy = bot_faction != shooter_faction;
		}
		else {
			is_enemy = bot_faction != shooter_faction && shooter_faction != faction_type::SPECTATOR;
		}

		if (!is_enemy) continue;

		const auto muzzle_pos = shot.muzzle_transform.pos;
		const auto raycast = physics.ray_cast_px(
			cosm.get_si(),
			character_pos,
			muzzle_pos,
			filter,
			character_handle
		);

		if (!raycast.hit) {
			const float dist = (muzzle_pos - character_pos).length();

			if (!player.ai_state.last_seen_target.is_set() || dist < current_target_distance) {
				player.ai_state.last_seen_target = shot.capability;
				player.ai_state.chase_remaining_time = 5.0f;
				player.ai_state.last_target_position = muzzle_pos;
				player.ai_state.has_dashed_for_last_seen_target = false;
				current_target_distance = dist;
			}
		}
	}

	if (auto* movement = character_handle.find<components::movement>()) {
		movement->flags.sprinting = false;
		movement->flags.dashing = false;
	}
} 