#pragma once
#include <cstddef>
#include "augs/templates/type_list.h"

template <bool...>
struct value_disjunction; 

template <>
struct value_disjunction<> {
	static constexpr bool value = false;
};

template <bool A, bool... Rest>
struct value_disjunction<A, Rest...> {
	static constexpr bool value = A || value_disjunction<Rest...>::value;
};

template <bool...>
struct value_conjunction; 

template <>
struct value_conjunction<> {
	static constexpr bool value = true;
};

template <bool A, bool... Rest>
struct value_conjunction<A, Rest...> {
	static constexpr bool value = A && value_conjunction<Rest...>::value;
};

template <std::size_t num_bytes, std::size_t alignment>
struct aligned_num_of_bytes {
	static constexpr std::size_t value = (((num_bytes - 1) / alignment) + 1) * alignment;
};

template <std::size_t alignment>
struct aligned_num_of_bytes<0, alignment> {
	static constexpr std::size_t value = 0;
};


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

template <std::size_t I, class List>
constexpr std::size_t sum_sizes_until_nth_v = sum_sizes_until_nth<I, 0, List>::value;

template <class List>
constexpr std::size_t sum_sizes_of_types_in_list_v = sum_sizes_until_nth<num_types_in_list_v<List> + 1, 0, List>::value;
