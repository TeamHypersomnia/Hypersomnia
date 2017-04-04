#pragma once
#include <array>
#include <unordered_map>
#include "game/enums/render_layer.h"

#include "game/components/wandering_pixels_component.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "augs/misc/delta.h"
#include "augs/misc/randomization.h"

class viewing_step;
struct visible_entities;

class interpolation_system;

namespace resources {
	struct particles_emission;
}

class wandering_pixels_system {
public:
	struct drawing_input : vertex_triangle_buffer_reference {
		using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;

		camera_cone camera;
		rgba colorize;
		renderable_drawing_type drawing_type = renderable_drawing_type::NORMAL;
	};

	struct particle {
		vec2 pos;
		vec2 current_direction = vec2(1, 0);
		float current_velocity = 20.f;
		float direction_ms_left = 0.f;
	};

	struct cache {
		components::wandering_pixels recorded_component;
		fast_randomization rng;

		std::vector<particle> particles;
		bool constructed = false;
	};

	double global_time_seconds = 0.0;

	//std::vector<cache> per_entity_cache;
	std::unordered_map<entity_id, cache> per_entity_cache;

	cache& get_cache(const const_entity_handle);
	const cache& get_cache(const const_entity_handle) const;

	void advance_for_visible(
		const visible_entities&,
		const cosmos&,
		const augs::delta dt
	);

	void advance_wandering_pixels_for(
		const const_entity_handle, 
		const augs::delta dt
	);

	void draw_wandering_pixels_for(
		const const_entity_handle, 
		const drawing_input&
	) const;

	void reserve_caches_for_entities(const size_t) const {}

	void erase_caches_for_dead_entities(const cosmos&);
};