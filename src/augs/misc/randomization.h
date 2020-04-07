#pragma once
#include <vector>

#include "augs/misc/randomization_declaration.h"

#include "augs/misc/bound.h"
#include "augs/math/vec2.h"
#include "augs/templates/variated.h"

#include "augs/misc/xorshift_state.h"

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

	uint32_t randval(
		uint32_t min, 
		uint32_t max
	);

	uint64_t randval(
		uint64_t min, 
		uint64_t max
	);

	template <class T>
	T make_guid();

	real32 randval(
		real32 min, 
		real32 max
	);

	real32 randval_v(
		real32 base_value, 
		real32 variation
	);

	real32 randval_vm(
		real32 base_value, 
		real32 variation_mult
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

	real32 randval_h(real32 bound);
	int randval_h(int bound);
	
	std::vector<real32> make_random_intervals(
		const std::size_t n, 
		const real32 maximum
	);

	std::vector<real32> make_random_intervals(
		const std::size_t n,
		const real32 maximum, 
		const real32 variation_multiplier
	);

	template <class A, class B>
	auto randval(const augs::simple_pair<A, B> p) {
		return randval(p.first, p.second);
	}

	template<class T>
	auto randval(const augs::random_bound<T> r) {
		const auto chosen_first = randval(r.first);
		const auto chosen_second = randval(r.second);
		return augs::bound<T>(chosen_first, chosen_second);
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
		const auto point = random_point_on_unit_circle<T>();
		const auto radius = randval(min_radius, max_radius);
		return point * radius;
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
		const auto x = randval(min_a.x, max_a.x);
		const auto y = randval(min_a.y, max_a.y);

		return { x, y };
	}

	template <class C>
	auto choose_from(C& container) {
		return container[randval(static_cast<std::size_t>(0), container.size() - 1)];
	}
};

struct randomization : basic_randomization<xorshift_state> {
	using basic_randomization<xorshift_state>::basic_randomization;
	static randomization from_random_device();
};
