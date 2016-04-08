#pragma once
#include "game_gui_root.h"
#include "game_framework/systems/gui_system.h"
#include "game_framework/systems/physics_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"
#include "game_framework/globals/filters.h"
#include "game_framework/components/name_component.h"

augs::entity_id game_gui_world::get_hovered_world_entity() {
	auto& physics = gui_system->parent_world.get_system<physics_system>();


	auto hovered = physics.query_aabb_px(gui_crosshair_position, gui_crosshair_position + vec2i(1, 1), filters::renderable_query());

	if(hovered.entities.size() > 0) {
		std::vector<augs::entity_id> hovered_and_named;

		for (auto h : hovered.entities)
			if (h->find<components::name>() != nullptr)
				hovered_and_named.push_back(h);

		if (hovered_and_named.size() > 0)
			return *hovered_and_named.begin();
	}

	return augs::entity_id();
}

