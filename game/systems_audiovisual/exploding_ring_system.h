#pragma once
#include "augs/misc/minmax.h"
#include "game/components/transform_component.h"
#include "augs/graphics/vertex.h"
#include "augs/misc/delta.h"
#include "game/messages/exploding_ring_input.h"

struct camera_cone;
class particles_simulation_system;

class exploding_ring_system {
public:
	struct ring {
		exploding_ring_input in;
		float time_of_occurence_seconds = 0.f;
	};

	float global_time_seconds = 0.f;

	std::vector<ring> rings;

	void acquire_new_rings(const std::vector<exploding_ring_input>& rings);

	void advance(
		const augs::delta dt,
		particles_simulation_system& particles_output_for_effects
	);

	void draw_rings(
		augs::vertex_triangle_buffer& triangles,
		augs::special_buffer& specials,
		const camera_cone camera,
		const cosmos& cosmos
	) const;

	void draw_highlights_of_rings(
		augs::vertex_triangle_buffer& triangles,
		const camera_cone camera,
		const cosmos& cosmos
	) const;

	void reserve_caches_for_entities(const size_t) const {}
};