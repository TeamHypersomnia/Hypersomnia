#pragma once
#include <cstddef>
#include "augs/templates/type_list.h"

template <std::size_t num_bytes, std::size_t alignment>
struct aligned_num_of_bytes {
	static constexpr std::size_t value = (((num_bytes - 1) / alignment) + 1) * alignment;
};

template <std::size_t alignment>
struct aligned_num_of_bytes<0, alignment> {
	static constexpr std::size_t value = 0;
};


template <class T, T... Vals>
struct constexpr_max;

template <class T>
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
{};


template <std::size_t I, std::size_t Current, class List, class = void>
struct sum_sizes_until_nth {
	static constexpr std::size_t value = 0;
};

template <std::size_t I, std::size_t Current, template <class...> class List, class T, class... Args>
struct sum_sizes_until_nth<I, Current, List<T, Args...>, std::enable_if_t<Current < I>>  {
	static constexpr std::size_t value = sizeof(T) + sum_sizes_until_nth<I, Current + 1, List<Args...>>::value;
};


template <std::size_t num_bytes, std::size_t alignment>
constexpr std::size_t aligned_num_of_bytes_v = aligned_num_of_bytes<num_bytes, alignment>::value;

template <class T, T First, T... Rest>
constexpr T constexpr_max_v = constexpr_max<T, First, Rest...>::value;

template <std::size_t I, class List>
constexpr std::size_t sum_sizes_until_nth_v = sum_sizes_until_nth<I, 0, List>::value;

template <class List>
constexpr std::size_t sum_sizes_of_types_in_list_v = sum_sizes_until_nth<num_types_in_list_v<List> + 1, 0, List>::value;
