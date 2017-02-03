#pragma once
#include <random>
#include <utility>
#include <vector>
#include "minmax.h"
#include "augs/math/declare.h"

struct randomization {
	std::mt19937 generator;
	randomization(size_t seed = 0);

	int randval(int min, int max);
	unsigned randval(unsigned min, unsigned max);
	float randval(float min, float max);
	float randval(float minmax);
	
	std::vector<float> make_random_intervals(size_t n, float maximum);
	std::vector<float> make_random_intervals(size_t n, float maximum, float variation_multiplier);

	unsigned randval(std::pair<unsigned, unsigned>);
	float randval(std::pair<float, float>);

	template<class T>
	vec2t<T> randval(const vec2t<T> min_a, vec2t<T> max_a) {
		return{ randval(min_a.x, max_a.x), randval(min_a.y, max_a.y) };
	}

	template<class T>
	T randval(augs::minmax<T> m) {
		return randval(m.first, m.second);
	}
};
