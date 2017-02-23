#pragma once
#include "augs/misc/minmax.h"
#include "game/components/transform_component.h"
#include "augs/graphics/vertex.h"
#include "augs/misc/delta.h"
#include "game/transcendental/entity_id.h"

struct camera_cone;

class pure_color_highlight_system {
public:
	struct highlight {
		struct input {
			float maximum_duration_seconds = 0.f;

			float starting_alpha_ratio = 0.f;

			entity_id target;
			rgba color;
		} in;
		
		float time_of_occurence_seconds = 0.f;
	};

	float global_time_seconds = 0.f;

	std::vector<highlight> highlights;
	
	void add(highlight::input);
	void advance(const augs::delta dt);
	
	void draw_highlights(
		augs::vertex_triangle_buffer& triangles,
		const camera_cone camera,
		const cosmos& cosmos,
		const interpolation_system& interp
	) const;

	void reserve_caches_for_entities(const size_t) const {}
};