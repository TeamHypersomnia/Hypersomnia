#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/visible_entities.h"
#include "game/detail/inventory/firearm_limit.h"
#include "game/inferred_caches/physics_world_cache.hpp"
#include "game/enums/filters.h"

/*
	A firearm lying on the ground that a bot would go for.
*/

template <class E>
bool is_retrievable_firearm(const E& item, const entity_id unreachable_item) {
	if (item.dead() || entity_id(item.get_id()) == unreachable_item) {
		return false;
	}

	if (!::is_firearm_like(item) || item.get_owning_transfer_capability().alive()) {
		return false;
	}

	if (const auto gun_def = item.template find<invariants::gun>(); gun_def != nullptr && gun_def->bots_ban) {
		return false;
	}

	/*
		Only items with the physics of lying items - not e.g. one flying after a throw.
	*/

	return ::calc_filters(item) == filters[predefined_filter_type::LYING_ITEM];
}

/*
	The nearest retrievable firearm within a screen around the bot (items are visible from anywhere).
*/

inline entity_id find_nearest_retrievable_firearm(
	const cosmos& cosm,
	const vec2 bot_pos,
	const entity_id unreachable_item
) {
	auto& entities = thread_local_visible_entities();

	entities.reacquire_all({
		cosm,
		camera_cone(camera_eye(bot_pos, 1.f), vec2i(1920, 1080)),
		accuracy_type::EXACT,
		render_layer_filter::whitelist(render_layer::ITEMS_ON_GROUND),
		{ { tree_of_npo_type::RENDERABLES } }
	});

	/* Important for the sake of determinism */
	entities.sort(cosm);

	auto nearest = entity_id();
	auto nearest_dist = 0.0f;

	entities.for_each<render_layer::ITEMS_ON_GROUND>(cosm, [&](const const_entity_handle& part) {
		const auto root = part.get_current_slot().alive() ? part.get_current_slot().get_root_container() : part;

		if (!::is_retrievable_firearm(root, unreachable_item)) {
			return;
		}

		const auto dist = (root.get_logic_transform().pos - bot_pos).length_sq();

		if (!nearest.is_set() || dist < nearest_dist) {
			nearest = root.get_id();
			nearest_dist = dist;
		}
	});

	return nearest;
}
