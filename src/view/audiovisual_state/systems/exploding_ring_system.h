#pragma once
#include "augs/misc/minmax.h"
#include "augs/misc/timing/delta.h"
#include "augs/math/camera_cone.h"

#include "augs/drawing/sprite.h"
#include "augs/drawing/drawing.h"

#include "game/messages/exploding_ring_input.h"
#include "game/components/transform_component.h"

class cosmos;
class particles_simulation_system;

class exploding_ring_system {
public:
	struct ring {
		exploding_ring_input in;
		double time_of_occurence_seconds = 0.0;
	};

	double global_time_seconds = 0.0;

	std::vector<ring> rings;

	void acquire_new_rings(const std::vector<exploding_ring_input>& rings);

	void advance(
		const cosmos& cosmos,
		const particle_effects_map&,
		const augs::delta dt,
		particles_simulation_system& particles_output_for_effects
	);

	void draw_rings(
		const augs::drawer_with_default output,
		augs::special_buffer& specials,
		const cosmos& cosmos,
		const camera_cone cone
	) const;

	void draw_highlights_of_rings(
		const augs::drawer output,
		const augs::atlas_entry highlight_tex,
		const cosmos& cosmos,
		const camera_cone cone
	) const;

	void reserve_caches_for_entities(const size_t) const {}
	void clear();
};