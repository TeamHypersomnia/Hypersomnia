#pragma once
#include <random>
#include <utility>

struct randomization {
	std::mt19937 generator;
	randomization(size_t seed = 0);

	int randval(int min, int max);
	unsigned randval(unsigned min, unsigned max);
	float randval(float min, float max);
	float randval(float minmax);

	unsigned randval(std::pair<unsigned, unsigned>);
	float randval(std::pair<float, float>);
};
