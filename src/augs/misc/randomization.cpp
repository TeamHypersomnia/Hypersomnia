#include <random>

#include "augs/math/repro_math.h"
#include "augs/misc/randomization.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/misc/xorshift.hpp"
#include "augs/log.h"
#include "3rdparty/crc32/crc32.h"

uint64_t next_seed(uint64_t& x) {
	uint64_t z = (x += 0x9e3779b97f4a7c15);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
	z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
	return z ^ (z >> 31);
}

uint64_t portable_hash(float x) {
	uint32_t i;
	std::memcpy(&i, &x, sizeof(i));
	return i;
}

uint64_t portable_hash(const std::string& x) {
	return portable_hash(crc32buf(reinterpret_cast<const char*>(x.data()), x.length()));
}

uint64_t portable_hash(uint64_t x) {
	return x;
}

uint64_t portable_hash(uint32_t x) {
	return x;
}

uint64_t portable_hash(int32_t x) {
	return x;
}

uint64_t portable_hash(uint16_t x) {
	return x;
}

template <class T>
basic_randomization<T>::basic_randomization(rng_seed_type seed) {
	for (int i = 0; i < 4; ++i) {
		generator.s[i] = next_seed(seed);
	}
}

randomization randomization::from_random_device() {
	return randomization(std::random_device()());
}

template <class generator_type>
template <class T>
T basic_randomization<generator_type>::make_guid() {
	if constexpr (std::is_same_v<uint64_t, T>) {
		return xoshiro256ss(&generator);
	}
	else {
		return randval(std::numeric_limits<T>::min(), std::numeric_limits<T>::max() - 1);
	}
}

template <class T>
int basic_randomization<T>::randval(
	const int min, 
	const int max
) {
	if (min == max) {
		return min;
	}

	const auto result = xoshiro256ss(&generator);
	return min + result % (max - min + 1);
}

template <class T>
uint32_t basic_randomization<T>::randval(
	const uint32_t min, 
	const uint32_t max
) {
	if (min == max) {
		return min;
	}

	const auto result = xoshiro256ss(&generator);
	return min + result % (max - min + 1);
}

template <class T>
real32 basic_randomization<T>::randval(
	const real32 min, 
	const real32 max
) {
	if (min == max) {
		return min;
	}

	const auto x = xoshiro256ss(&generator);
	const auto u01 = (x >> 11) * 0x1.0p-53;
	return min + static_cast<real32>(u01) * (max - min);
}

template <class T>
uint64_t basic_randomization<T>::randval(
	const uint64_t min, 
	const uint64_t max
) {
	if (min == max) {
		return min;
	}

	const auto result = xoshiro256ss(&generator);
	return min + result % (max - min + 1);
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

template struct basic_randomization<xorshift_state>;

template uint32_t basic_randomization<xorshift_state>::make_guid<uint32_t>();
template uint64_t basic_randomization<xorshift_state>::make_guid<uint64_t>();