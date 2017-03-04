#pragma once
#include <vector>
#include <chrono>

#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"
#include "augs/templates/maybe_const.h"

struct neon_map_stamp {
	float standard_deviation = 0.f;
	unsigned radius_towards_x_axis = 0xdeadbeef;
	unsigned radius_towards_y_axis = 0xdeadbeef;
	float amplification = 0.f;
	std::chrono::system_clock::time_point last_write_time_of_source;

	std::vector<rgba> light_colors;
};

namespace augs {
	template <
		bool C,
		class F
	>
	auto introspect(
		maybe_const_ref_t<C, neon_map_stamp> data,
		F f
	) {
		return
			f(data.standard_deviation)
			&& f(data.radius_towards_x_axis)
			&& f(data.radius_towards_y_axis)
			&& f(data.amplification)
			&& f(data.last_write_time_of_source)
			&& f(data.light_colors)
		;
	}
}

void regenerate_neon_maps();