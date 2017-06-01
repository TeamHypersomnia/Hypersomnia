#pragma once
#include <cstddef>

template <std::size_t num_bytes, std::size_t alignment>
struct aligned_num_of_bytes {
	static constexpr std::size_t value = (((num_bytes - 1) / alignment) + 1) * alignment;
};

template <std::size_t alignment>
struct aligned_num_of_bytes<0, alignment> {
	static constexpr std::size_t value = 0;
};

template <std::size_t num_bytes, std::size_t alignment>
constexpr std::size_t aligned_num_of_bytes_v = aligned_num_of_bytes<num_bytes, alignment>::value;

static_assert(aligned_num_of_bytes_v<0, 4> == 0, "Trait is wrong");
static_assert(aligned_num_of_bytes_v<1, 4> == 4, "Trait is wrong");
static_assert(aligned_num_of_bytes_v<2, 4> == 4, "Trait is wrong");
static_assert(aligned_num_of_bytes_v<3, 4> == 4, "Trait is wrong");
static_assert(aligned_num_of_bytes_v<4, 4> == 4, "Trait is wrong");
static_assert(aligned_num_of_bytes_v<5, 4> == 8, "Trait is wrong");
static_assert(aligned_num_of_bytes_v<6, 4> == 8, "Trait is wrong");
static_assert(aligned_num_of_bytes_v<7, 4> == 8, "Trait is wrong");
static_assert(aligned_num_of_bytes_v<8, 4> == 8, "Trait is wrong");
static_assert(aligned_num_of_bytes_v<9, 4> == 12, "Trait is wrong");

template<class T, T... Vals>
struct constexpr_max;

template<class T>
struct constexpr_max<T>
{	// maximum of nothing is 0
	static constexpr T value = static_cast<T>(0);
};

template<class T, T Val>
struct constexpr_max<T, Val>
{	// maximum of Val is Val
	static constexpr T value = Val;
};

template <class T, T First, T Second, T... Rest>
struct constexpr_max<T, First, Second, Rest...> : constexpr_max<T, (First < Second ? Second : First), Rest...>
{	// find maximum value in First, Second, Rest...
};

template <class T, T First, T Second, T... Rest>
constexpr T constexpr_max_v = constexpr_max<T, First, Second, Rest...>::value;