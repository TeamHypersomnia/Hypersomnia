#pragma once
#include <vector>
#include <chrono>

#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"

struct neon_map_metadata {
	float standard_deviation = 0.f;
	unsigned radius_towards_x_axis = 0xdeadbeef;
	unsigned radius_towards_y_axis = 0xdeadbeef;
	float amplification = 0.f;
	std::chrono::system_clock::time_point last_write_time_of_source;

	std::vector<rgba> light_colors;
};

namespace augs {
	template <class A>
	bool read_object(A& ar, neon_map_metadata& data) {
		return
			read_object(ar, data.standard_deviation)
			&& read_object(ar, data.radius_towards_x_axis)
			&& read_object(ar, data.radius_towards_y_axis)
			&& read_object(ar, data.amplification)
			&& read_object(ar, data.last_write_time_of_source)
			&& read_object(ar, data.light_colors)
			;
	}

	template <class A>
	void write_object(A& ar, const neon_map_metadata& data) {
		write_object(ar, data.standard_deviation);
		write_object(ar, data.radius_towards_x_axis);
		write_object(ar, data.radius_towards_y_axis);
		write_object(ar, data.amplification);
		write_object(ar, data.last_write_time_of_source);
		write_object(ar, data.light_colors);
	}
}

void regenerate_neon_maps();