#include "randomization.h"
#include "augs/templates/container_templates.h"
#include <algorithm>

template <class T>
basic_randomization<T>::basic_randomization(const size_t seed) {
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
float basic_randomization<T>::randval(const float minmax) {
	return randval(-minmax, minmax);
}

template <class T>
std::vector<float> basic_randomization<T>::make_random_intervals(
	const size_t n, 
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
	const size_t n, 
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

	sort_container(result);

	return result;
}

template struct basic_randomization<std::mt19937>;
template struct basic_randomization<std::minstd_rand0>;