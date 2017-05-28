#pragma once
#include <cstddef>
#include "augs/templates/constexpr_if.h"
#include "augs/ensure.h"

template <
	class T, 
	std::size_t current_candidate = 0u,
	class F, 
	class... Types
>
void dynamic_dispatch_impl(
	T&& index_gettable_object, 
	const std::size_t dynamic_type_index, 
	F&& generic_call
) {
	augs::constexpr_if<current_candidate < sizeof...(Types)>()(
		[dynamic_type_index](auto&& gettable, auto&& call) {
			if (current_candidate == dynamic_type_index) {
				call(std::get<current_candidate>(gettable));
			}
			else {
				dynamic_dispatch_impl<T, current_candidate + 1, F, Types...>(
					std::forward<T>(gettable),
					in,
					std::forward<F>(call)
				);
			}
		},
		std::forward<T>(index_gettable_object), 
		std::forward<F>(generic_call)
	)._else(
		[dynamic_type_index](auto...){
			LOG_NVPS(dynamic_type_index);
			ensure(false && "dynamic_type_index is out of bounds!");
		}	
	);
}

template <
	template <class...> class T, 
	class F,
	class... Types
>
void dynamic_dispatch(T<Types...>& obj, const std::size_t dynamic_type_index, F&& call) {
	dynamic_dispatch_impl<T<Types...>, 0, F, Types...>(obj, dynamic_type_index, std::forward<F>(call));
}

template <
	template <class...> class T, 
	class F,
	class... Types
>
void dynamic_dispatch(const T<Types...>& obj, F&& call) {
	dynamic_dispatch<const T<Types...>, 0, F, Types...>(obj, dynamic_type_index, std::forward<F>(call));
}