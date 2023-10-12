#pragma once
#include "augs/templates/type_list.h"
#include "augs/templates/type_pair.h"
#include "augs/templates/filter_types.h"

template <class L>
struct type_map_impl {
	template <class T>
	struct at_impl {
		template <class Candidate>
		struct match_first : std::bool_constant<std::is_same_v<typename Candidate::First, T>> {};

		using type = typename find_matching_type_in_list<match_first, L>::Second;
	};

	template <class S>
	using at = typename at_impl<S>::type;
};

template <class... Pairs>
using type_map = type_map_impl<type_list<Pairs...>>;

struct always_passes_type {};

template <class ValueType, class L>
struct type_to_value_map_impl {
	template <class T>
	struct at_impl {
		template <class Candidate>
		struct match_first : std::bool_constant<
			std::is_same_v<typename Candidate::First, T> || 
			std::is_same_v<typename Candidate::First, always_passes_type>
		> {};

		static constexpr ValueType value = find_matching_type_in_list<match_first, L>::Value;
	};

	template <class S>
	static constexpr ValueType at = at_impl<S>::value;
};

template <class ValueType, ValueType DefaultValue, class... Pairs>
using type_value_map = type_to_value_map_impl<ValueType, type_list<Pairs..., type_value_pair<always_passes_type, ValueType, DefaultValue>>>;

