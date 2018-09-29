#pragma once
#include "augs/misc/timing/delta.h"
#include "augs/misc/minmax.h"

#include "augs/graphics/vertex.h"

#include "view/viewables/all_viewables_declaration.h"
#include "game/cosmos/entity_id.h"
#include "game/components/transform_component.h"

class cosmos;
class interpolation_system;

namespace augs {
	struct drawer;
}


struct draw_renderable_input;

class pure_color_highlight_system {
public:
	struct highlight {
		struct input {
			float maximum_duration_seconds = 0.f;

			float starting_alpha_ratio = 0.f;

			entity_id target;
			rgba color = white;
		} in;
		
		double time_of_occurence_seconds = 0.0;
	};

	double global_time_seconds = 0.0;

	std::vector<highlight> highlights;
	
	void add(highlight::input);
	void advance(const augs::delta dt);
	
	void draw_highlights(
		const cosmos& cosm,
		const draw_renderable_input&
	) const;

	void reserve_caches_for_entities(const size_t) const {}
	void clear();
};