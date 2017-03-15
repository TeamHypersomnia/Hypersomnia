#pragma once
#include <type_traits>

template <template <class...> class... Types>
struct template_disjunction {
	template <class T>
	struct predicate {
		static constexpr bool value = std::disjunction_v<Types<T>...>;
	};
};