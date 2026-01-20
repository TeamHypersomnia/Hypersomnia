#pragma once
#include "game/modes/ai/behaviors/ai_behavior_plant.hpp"
#include "game/modes/ai/behaviors/ai_behavior_process_ctx.hpp"
#include "game/components/sentience_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/modes/ai/arena_mode_ai_structs.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/explosive/like_explosive.h"

/*
	Implementation of ai_behavior_plant::process().
	
	NOTE: Weapon wielding for planting (pulling out bomb) is handled statelessly via
	should_holster_weapons() and calc_wielding_intent().
	
	NOTE: Crosshair offset and hand_flags are calculated statelessly
	via calc_movement_and_crosshair() and calc_hand_flags().
*/

inline void ai_behavior_plant::process(ai_behavior_process_ctx& ctx) {
	auto& cosm = ctx.cosm;
	const auto bomb_entity = ctx.bomb_entity;
	const bool pathfinding_just_completed = ctx.pathfinding_just_completed;

	const auto character_handle = cosm[ctx.controlled_character_id];

	if (!character_handle.alive()) {
		return;
	}

	/*
		Lambda to reset plant state and choose a new bombsite target.
	*/
	auto reset_plant_state = [this]() {
		is_planting = false;
		cached_plant_target.reset();
	};

	/*
		If is_planting, check if the bomb's when_started_arming is still set.
		If not, the plant was interrupted (e.g. bot moved or was pushed).
	*/
	if (is_planting && bomb_entity.is_set()) {
		const auto bomb_handle = cosm[bomb_entity];

		if (bomb_handle.alive()) {
			/*
				Check if the bomb is still held by this bot and arming is still in progress.
			*/
			const auto bomb_owner = bomb_handle.get_owning_transfer_capability();

			if (bomb_owner != character_handle) {
				/*
					Bomb is no longer held by this bot - plant was interrupted.
				*/
				reset_plant_state();
				return;
			}

			if (const auto* hand_fuse = bomb_handle.find<components::hand_fuse>()) {
				if (!hand_fuse->when_started_arming.was_set()) {
					/*
						Plant was interrupted - when_started_arming is unset.
						Reset plant state and choose a new random target.
					*/
					AI_LOG("Plant interrupted - when_started_arming unset");
					reset_plant_state();
					return;
				}
			}
		}
		else {
			/*
				Bomb entity is dead - shouldn't happen but handle gracefully.
			*/
			reset_plant_state();
			return;
		}
	}

	if (!is_planting && pathfinding_just_completed) {
		AI_LOG("Reached bombsite - starting plant");
		is_planting = true;
	}
}
