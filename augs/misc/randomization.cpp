#include "randomization.h"
#include "augs/templates/container_templates.h"
#include <algorithm>

randomization::randomization(size_t seed) {
	generator.seed(seed);
}

int randomization::randval(int min, int max) {
	if (min == max) return min;
	return std::uniform_int_distribution<int>(min, max)(generator);
}

unsigned randomization::randval(unsigned min, unsigned max) {
	if (min == max) return min;
	return std::uniform_int_distribution<unsigned>(min, max)(generator);
}

float randomization::randval(float min, float max) {
	if (min == max) return min;
	return std::uniform_real_distribution<float>(min, max)(generator);
}

float randomization::randval(float minmax) {
	return randval(-minmax, minmax);
}

std::vector<float> randomization::make_random_intervals(const size_t n, const float maximum) {
	std::vector<float> result;
	result.resize(n);

	for (size_t i = 0; i < n; ++i) {
		result[i] = randval(0.f, maximum);
	}

	std::sort(result.begin(), result.end());

	return std::move(result);
}

std::vector<float> randomization::make_random_intervals(const size_t n, const float maximum, const float variation_multiplier) {
	std::vector<float> result;
	result.resize(n);

	const float interval_length = maximum / n;

	for (size_t i = 0; i < n; ++i) {
		result[i] = interval_length * i + randval(-variation_multiplier, variation_multiplier) * interval_length;
		result[i] = std::min(maximum, result[i]);
		result[i] = std::max(0.f, result[i]);
	}

	sort_container(result);

	return std::move(result);
}

unsigned randomization::randval(std::pair<unsigned, unsigned> p) {
	return randval(p.first, p.second);
}

float randomization::randval(std::pair<float, float> p) {
	return randval(p.first, p.second);
}