#include "augs/math/repro_math.h"
#include "augs/misc/randomization.h"
#include "augs/templates/algorithm_templates.h"

template <class T>
basic_randomization<T>::basic_randomization(const rng_seed_type seed) {
	streflop::RandomInit(seed, generator);
}

template <class T>
int basic_randomization<T>::randval(
	const int min, 
	const int max
) {
	if (min == max) {
		return min;
	}

	return streflop::RandomII<int>(min, max, generator);
}

template <class T>
unsigned basic_randomization<T>::randval(
	const unsigned min, 
	const unsigned max
) {
	if (min == max) {
		return min;
	}

	return streflop::RandomII<unsigned>(min, max, generator);
}

template <class T>
real32 basic_randomization<T>::randval(
	const real32 min, 
	const real32 max
) {
	if (min == max) {
		return min;
	}

	return streflop::RandomII<real32>(min, max, generator);
}

template <class T>
std::size_t basic_randomization<T>::randval(
	std::size_t min, 
	std::size_t max
) {
	if (min == max) {
		return min;
	}

	return streflop::RandomII<std::size_t>(min, max, generator);
}

template <class T>
real32 basic_randomization<T>::randval_h(const real32 h) {
	return randval(-h, h);
}

template <class T>
real32 basic_randomization<T>::randval_v(
	real32 base_value, 
	real32 variation
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
real32 basic_randomization<T>::randval_vm(
	real32 base_value, 
	real32 variation_mult
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
std::vector<real32> basic_randomization<T>::make_random_intervals(
	const std::size_t n, 
	const real32 maximum
) {
	std::vector<real32> result;
	result.resize(n);

	for (size_t i = 0; i < n; ++i) {
		result[i] = randval(0.f, maximum);
	}

	std::sort(result.begin(), result.end());

	return result;
}

template <class T>
std::vector<real32> basic_randomization<T>::make_random_intervals(
	const std::size_t n, 
	const real32 maximum, 
	const real32 variation_multiplier
) {
	std::vector<real32> result;
	result.resize(n);

	const real32 interval_length = maximum / n;

	for (size_t i = 0; i < n; ++i) {
		result[i] = interval_length * i + randval(-variation_multiplier, variation_multiplier) * interval_length;
		result[i] = std::min(maximum, result[i]);
		result[i] = std::max(0.f, result[i]);
	}

	sort_range(result);

	return result;
}

template struct basic_randomization<streflop::RandomState>;