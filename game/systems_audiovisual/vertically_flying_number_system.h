#pragma once
#include "augs/gui/text_drawer.h"
#include "augs/math/vec2.h"
#include "augs/graphics/vertex.h"

struct camera_cone;

class vertically_flying_number_system {
public:
	struct number {
		struct input {
			augs::gui::text_drawer text;
			
			float maximum_duration_seconds = 0.f;
			
			float value = 0.f;
			vec2 pos;
		} in;

		float time_of_occurence_seconds = 0.f;
	};

	float global_time_seconds = 0.f;

	std::vector<number> numbers;

	void reserve_caches_for_entities(const size_t) const {}

	void add(number::input);
	void advance(const augs::delta dt);

	void draw_numbers(
		augs::vertex_triangle_buffer& triangles,
		const camera_cone
	) const;
};