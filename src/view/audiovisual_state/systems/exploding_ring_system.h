#pragma once
#include "augs/misc/minmax.h"
#include "augs/misc/timing/delta.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/math/camera_cone.h"

#include "augs/drawing/sprite.h"
#include "augs/drawing/drawing.h"

#include "game/messages/exploding_ring_input.h"
#include "game/components/transform_component.h"

#include "view/view_container_sizes.h"

class cosmos;
class particles_simulation_system;
struct randomization;

class exploding_ring_system {
public:
	struct ring {
		exploding_ring_input in;
		double time_of_occurence_seconds = 0.0;
	};

	double global_time_seconds = 0.0;

	augs::constant_size_vector<ring, MAX_EXPLODING_RINGS> rings;

	template <class C>
	void acquire_new_rings(const C& rings);
	
	template <class I>
	void acquire_new_ring(I&&);

	void advance(
		randomization& rng,
		const cosmos& cosm,
		const particle_effects_map&,
		const augs::delta dt,
		particles_simulation_system& particles_output_for_effects
	);

	void draw_rings(
		const augs::drawer_with_default output,
		augs::special_buffer& specials,
		const cosmos& cosm,
		const camera_cone cone
	) const;

	void draw_highlights_of_rings(
		const augs::drawer output,
		const augs::atlas_entry highlight_tex,
		const cosmos& cosm,
		const camera_cone cone
	) const;

	void reserve_caches_for_entities(const size_t) const {}
	void clear();
};