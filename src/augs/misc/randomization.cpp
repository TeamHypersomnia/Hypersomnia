#include "augs/misc/randomization.h"
#include "augs/templates/algorithm_templates.h"

template <class T>
basic_randomization<T>::basic_randomization(const rng_seed_type seed) {
	generator.seed(seed);
}

template <class T>
int basic_randomization<T>::randval(
	const int min, 
	const int max
) {
	if (min == max) {
		return min;
	}

	return std::uniform_int_distribution<int>(min, max)(generator);
}

template <class T>
unsigned basic_randomization<T>::randval(
	const unsigned min, 
	const unsigned max
) {
	if (min == max) {
		return min;
	}

	return std::uniform_int_distribution<unsigned>(min, max)(generator);
}

template <class T>
float basic_randomization<T>::randval(
	const float min, 
	const float max
) {
	if (min == max) {
		return min;
	}

	return std::uniform_real_distribution<float>(min, max)(generator);
}

template <class T>
std::size_t basic_randomization<T>::randval(
	std::size_t min, 
	std::size_t max
) {
	if (min == max) {
		return min;
	}

	return std::uniform_int_distribution<std::size_t>(min, max)(generator);
}

template <class T>
float basic_randomization<T>::randval_h(const float h) {
	return randval(-h, h);
}

template <class T>
float basic_randomization<T>::randval_v(
	float base_value, 
	float variation
) {
	return randval(base_value - variation, base_value + variation);
}

template <class T>
int basic_randomization<T>::randval_v(
	int base_value, 
	int variation
) {
	return randval(base_value - variation, base_value + variation);
}

template <class T>
float basic_randomization<T>::randval_vm(
	float base_value, 
	float variation_mult
) {
	const auto h = (base_value * variation_mult) / 2;
	const auto result = randval(base_value - h, base_value + h);
	return result;
}

template <class T>
int basic_randomization<T>::randval_h(const int h) {
	return randval(-h, h);
}

template <class T>
std::vector<float> basic_randomization<T>::make_random_intervals(
	const std::size_t n, 
	const float maximum
) {
	std::vector<float> result;
	result.resize(n);

	for (size_t i = 0; i < n; ++i) {
		result[i] = randval(0.f, maximum);
	}

	std::sort(result.begin(), result.end());

	return result;
}

template <class T>
std::vector<float> basic_randomization<T>::make_random_intervals(
	const std::size_t n, 
	const float maximum, 
	const float variation_multiplier
) {
	std::vector<float> result;
	result.resize(n);

	const float interval_length = maximum / n;

	for (size_t i = 0; i < n; ++i) {
		result[i] = interval_length * i + randval(-variation_multiplier, variation_multiplier) * interval_length;
		result[i] = std::min(maximum, result[i]);
		result[i] = std::max(0.f, result[i]);
	}

	sort_range(result);

	return result;
}

template struct basic_randomization<std::mt19937>;
template struct basic_randomization<std::minstd_rand0>;