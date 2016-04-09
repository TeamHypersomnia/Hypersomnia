#pragma once
#include "game_gui_root.h"
#include "game_framework/systems/gui_system.h"
#include "game_framework/systems/physics_system.h"
#include "entity_system/world.h"
#include "entity_system/entity.h"
#include "game_framework/globals/filters.h"
#include "game_framework/components/name_component.h"

augs::entity_id game_gui_world::get_hovered_world_entity(vec2 camera_pos) {
	auto& physics = gui_system->parent_world.get_system<physics_system>();

	auto cursor_pointing_at = camera_pos + gui_crosshair_position - size / 2;

	std::vector<vec2> v{ cursor_pointing_at, cursor_pointing_at + vec2(1, 0), cursor_pointing_at + vec2(1, 1) , cursor_pointing_at + vec2(0, 1) };
	auto hovered = physics.query_polygon(v, filters::renderable_query());

	if(hovered.entities.size() > 0) {
		std::vector<augs::entity_id> hovered_and_named;

		for (auto h : hovered.entities) {
			auto named = get_first_named_ancestor(h);
			
			if (named.alive())
				hovered_and_named.push_back(named);
		}

		if (hovered_and_named.size() > 0)
			return *hovered_and_named.begin();
	}

	return augs::entity_id();
}

