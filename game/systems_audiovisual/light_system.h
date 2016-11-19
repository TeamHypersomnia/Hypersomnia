#pragma once
#include "augs/graphics/vertex.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/graphics/renderer.h"

#include "augs/misc/randomization.h"

class viewing_step;

class light_system {
public:
	struct cache {
		std::array<float, 10> all_variation_values;
		cache();
	};

	std::vector<cache> per_entity_cache;

	randomization rng;

	void reserve_caches_for_entities(const size_t);

	void render_all_lights(augs::renderer& output, std::array<float, 16> projection_matrix, viewing_step&);
};