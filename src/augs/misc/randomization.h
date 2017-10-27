#pragma once
#include <random>
#include <utility>
#include <vector>

#include "augs/misc/minmax.h"
#include "augs/math/vec2.h"

template <class generator_type>
struct basic_randomization {
	generator_type generator;

	basic_randomization(const size_t seed = 0);

	int randval(
		const int min, 
		const int max
	);

	unsigned randval(
		const unsigned min, 
		const unsigned max
	);

	float randval(
		const float min, 
		const float max
	);

	float randval(const float minmax);
	
	std::vector<float> make_random_intervals(
		const size_t n, 
		const float maximum
	);

	std::vector<float> make_random_intervals(
		const size_t n, 
		const float maximum, 
		const float variation_multiplier
	);

	template<class A, class B>
	auto randval(const std::pair<A, B> p) {
		return randval(p.first, p.second);
	}

	template<class T>
	basic_vec2<T> random_point_in_ring(
		const T min_radius,
		const T max_radius
	) {
		return randval(min_radius, max_radius) * vec2().set_from_degrees(
			randval(
				static_cast<T>(0), 
				static_cast<T>(360)
			));
	}

	template<class T>
	basic_vec2<T> random_point_in_circle(
		const T max_radius
	) {
		return random_point_in_ring(static_cast<T>(0), max_radius);
	}

	template<class T>
	basic_vec2<T> randval(
		const basic_vec2<T> min_a, 
		const basic_vec2<T> max_a
	) {
		return { 
			randval(min_a.x, max_a.x), 
			randval(min_a.y, max_a.y) 
		};
	}

	template<class T>
	T randval(const augs::minmax<T> m) {
		return randval(m.first, m.second);
	}
};

using randomization = basic_randomization<std::mt19937>;
using fast_randomization = basic_randomization<std::minstd_rand0>;
