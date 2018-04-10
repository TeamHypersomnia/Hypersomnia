#pragma once
#include "augs/misc/timing/delta.h"
#include "augs/misc/minmax.h"

#include "augs/graphics/vertex.h"

#include "view/viewables/all_viewables_declarations.h"
#include "game/transcendental/entity_id.h"
#include "game/components/transform_component.h"

class cosmos;
class interpolation_system;

namespace augs {
	struct drawer;
}

class pure_color_highlight_system {
public:
	struct highlight {
		struct input {
			float maximum_duration_seconds = 0.f;

			float starting_alpha_ratio = 0.f;

			entity_id target;
			rgba color;
		} in;
		
		double time_of_occurence_seconds = 0.0;
	};

	double global_time_seconds = 0.0;

	std::vector<highlight> highlights;
	
	void add(highlight::input);
	void advance(const augs::delta dt);
	
	void draw_highlights(
		const augs::drawer output,
		const cosmos& cosmos,
		const interpolation_system& interp,
		const images_in_atlas_map& game_images
	) const;

	void reserve_caches_for_entities(const size_t) const {}
	void clear();
};