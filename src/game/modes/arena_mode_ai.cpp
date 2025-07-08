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

arena_ai_result update_arena_mode_ai(
	const arena_mode::input& in,
	const logic_step step,
	arena_mode_player& player,
	const bool is_ffa,
	xorshift_state& stable_round_rng
) {
	auto stable_rng = randomization(stable_round_rng);

	auto scope = augs::scope_guard([&]() {
		stable_round_rng = stable_rng.generator;
	});

	const auto money = player.stats.money;
	auto& cosm = in.cosm;
	const auto character_handle = cosm[player.controlled_character_id];
	
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
		const auto seed = cosm.get_rng_seed_for(player.controlled_character_id);
		auto rng = randomization(seed);
		
		player.ai_state.movement_duration_secs = rng.randval(1.0f, 3.0f);
		player.ai_state.current_movement_direction = rng.random_point_in_unit_circle<real32>();
		player.ai_state.movement_timer_remaining = player.ai_state.movement_duration_secs;
	}

	if (player.ai_state.last_seen_target.is_set()) {
		const auto distance_to_last_seen = (player.ai_state.last_target_position - character_pos).length();
		const auto reached_threshold = 100.0f; // Reset chase when within 50 units
		
		// Update chase timer using delta time
		player.ai_state.chase_remaining_time -= dt_secs;
		
		if (player.ai_state.chase_remaining_time > 0.0f && distance_to_last_seen > reached_threshold) {
			// Chase to last seen position
			actual_movement_direction = (player.ai_state.last_target_position - character_pos).normalize();
		}
		else {
			// Reset chase if reached target or timeout
			if (distance_to_last_seen <= reached_threshold) {
				player.ai_state.last_seen_target = entity_id::dead();
				player.ai_state.chase_remaining_time = 0.0f;
			}
			// Resume random movement
			actual_movement_direction = player.ai_state.current_movement_direction;
		}
	}
	else {
		// Random movement when no target
		actual_movement_direction = player.ai_state.current_movement_direction;
	}

	// Simple target detection - look for enemies in the same faction
	entity_id closest_enemy = entity_id::dead();
	float closest_distance = std::numeric_limits<float>::max();

	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto filter = predefined_queries::pathfinding();

	// Get current aim direction from actual crosshair for line of sight
	vec2 current_aim_direction = vec2::zero;
	if (auto crosshair = character_handle.find_crosshair()) {
		current_aim_direction = crosshair->base_offset.normalize();
	}
	else {
		// Fallback to movement direction if no crosshair
		current_aim_direction = player.ai_state.current_movement_direction;
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

			const auto distance = (entity.get_logic_transform().pos - character_pos).length();
			
			if (distance < closest_distance && distance < 1080.0f * 2) {
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

	if (closest_enemy.is_set()) {
		player.ai_state.last_seen_target = closest_enemy;
		player.ai_state.chase_remaining_time = 5.0f; // 5 second chase timeout
		player.ai_state.last_target_position = cosm[closest_enemy].get_logic_transform().pos;
	}

	const bool has_target = 
		closest_enemy.is_set() && 
		cosm[closest_enemy].alive() &&
		::sentient_and_conscious(cosm[closest_enemy])
	;

	const bool target_in_range = has_target && closest_distance < 500.0f;

	// Set target crosshair offset based on current situation
	if (target_in_range) {
		const auto target_pos = cosm[closest_enemy].get_logic_transform().pos;
		const auto aim_direction = (target_pos - character_pos);
		player.ai_state.target_crosshair_offset = aim_direction;

		if (auto sentience = character_handle.find<components::sentience>()) {
			sentience->hand_flags[0] = true;
		}
	}
	else {
		// Aim in the direction of movement when not shooting
		player.ai_state.target_crosshair_offset = actual_movement_direction * 200.0f;

		if (auto sentience = character_handle.find<components::sentience>()) {
			sentience->hand_flags[0] = false;
		}
	}

	// Smoothly interpolate crosshair towards target offset
	if (auto crosshair = character_handle.find_crosshair()) {
		const float average_factor = 0.5f;
		const float averages_per_sec = target_in_range ? 0.4f : 0.1f;
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

			int weapon_count = 0;
			std::vector<item_flavour_id> owned_guns;
			
			character_handle.for_each_contained_item_recursive(
				[&](const auto& item) {
					if (const auto gun = item.template find<invariants::gun>()) {
						weapon_count++;
						owned_guns.push_back(item.get_flavour_id());
					}
				}
			);

			// Try to buy a gun if we have only one weapon
			if (weapon_count <= 1) {
				std::vector<item_flavour_id> affordable_guns;
				
				cosm.for_each_flavour_having<invariants::gun>([&](const auto& id, const auto&) {
					const auto price = *::find_price_of(cosm, item_flavour_id(id));

					if (price <= money) {
						affordable_guns.push_back(item_flavour_id(id));
					}
				});

				if (!affordable_guns.empty()) {
					// Pick a random affordable gun
					const auto random_index = stable_rng.randval(0u, static_cast<unsigned>(affordable_guns.size() - 1));
					const auto bought = affordable_guns[random_index];

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

	// Set movement flags at the very end
	movement.flags.set_from_closest_direction(actual_movement_direction);
	
	return result;
} 