#pragma once
#include <optional>

#include "augs/math/vec2.h"
#include "augs/misc/timing/delta.h"

#include "augs/graphics/rgba.h"
#include "augs/math/camera_cone.h"

namespace augs {
	struct baked_font;
	struct drawer;
}

class flying_number_indicator_system {
public:
	struct number {
		struct input {
			std::string text;
			rgba color;
			
			float maximum_duration_seconds = 0.f;

			vec2 impact_velocity;
			vec2 pos;
		};

		input in;
		double time_of_occurence_seconds = 0.0;

		mutable std::optional<vec2> first_camera_space_pos;
	};

	double global_time_seconds = 0.0;

	std::vector<number> numbers;

	void reserve_caches_for_entities(const size_t) const {}
	void clear();

	void add(const number::input);
	void advance(const augs::delta dt);

	void draw_numbers(
		const augs::baked_font& font,
		const augs::drawer output,
		const camera_cone
	) const;
};