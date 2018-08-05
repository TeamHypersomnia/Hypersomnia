#pragma once
#include <random>
#include <vector>

#include "augs/misc/randomization_declaration.h"

#include "augs/misc/minmax.h"
#include "augs/math/vec2.h"
#include "augs/templates/variated.h"

template <class generator_type>
struct basic_randomization {
	generator_type generator;

	basic_randomization(const rng_seed_type seed = 0);

	int randval(
		int min, 
		int max
	);

	int randval_v(
		int base_value, 
		int variation
	);

	unsigned randval(
		unsigned min, 
		unsigned max
	);

	std::size_t randval(
		std::size_t min, 
		std::size_t max
	);

	float randval(
		float min, 
		float max
	);

	float randval_v(
		float base_value, 
		float variation
	);

	float randval_vm(
		float base_value, 
		float variation_mult
	);

	template <class T>
	auto randval(const augs::variated<T>& v) {
		if constexpr(std::is_same_v<T, unsigned>) {
			return static_cast<unsigned>(randval_v(static_cast<int>(v.value), static_cast<int>(v.variation)));
		}
		else {
			return randval_v(v.value, v.variation);
		}
	}

	template <class T>
	auto randval(const augs::mult_variated<T>& v) {
		return randval_vm(v.value, v.variation);
	}

	float randval_h(float minmax);
	int randval_h(int minmax);
	
	std::vector<float> make_random_intervals(
		const std::size_t n, 
		const float maximum
	);

	std::vector<float> make_random_intervals(
		const std::size_t n,
		const float maximum, 
		const float variation_multiplier
	);

	template <class A, class B>
	auto randval(const augs::simple_pair<A, B> p) {
		return randval(p.first, p.second);
	}

	template<class T>
	auto randval(const augs::random_bound<T> r) {
		return augs::minmax<T>(randval(r.first), randval(r.second));
	}

	template <class T>
	basic_vec2<T> random_point_on_unit_circle() {
		const auto random_angle = randval(static_cast<T>(0), static_cast<T>(360));
		return basic_vec2<T>::from_degrees(random_angle);
	}

	template<class T>
	basic_vec2<T> random_point_in_ring(
		const T min_radius,
		const T max_radius
	) {
		return randval(min_radius, max_radius) * random_point_on_unit_circle<T>();
	}

	template <class T>
	basic_vec2<T> random_point_in_unit_circle() {
		return random_point_in_ring(static_cast<T>(0), static_cast<T>(1));
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

	template <class C>
	auto choose_from(C& container) {
		return container[randval(static_cast<std::size_t>(0), container.size() - 1)];
	}
};

struct randomization : basic_randomization<std::mt19937> {
	using basic_randomization<std::mt19937>::basic_randomization;
};

struct fast_randomization : basic_randomization<std::minstd_rand0> {
	using basic_randomization<std::minstd_rand0>::basic_randomization;
};
