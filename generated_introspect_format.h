#pragma once
#include <tuple>
#include <array>

#include "augs/misc/trivially_copyable_tuple.h"
#include "augs/templates/for_each_in_types.h"

#include "game/transcendental/entity_id_declaration.h"
#include "game/detail/shape_variant_declaration.h"

#define FIELD(x) f(#x, _t_.x...)

/* Other introspectors that do not fit into the standard schema go here: */

namespace augs {
	template <class F, class ElemType, size_t count, class... Instances>
	void introspect_body(
		const std::array<ElemType, count>* const,
		F f,
		Instances&&... t
	) {
		for (size_t i = 0; i < count; ++i) {
			f(std::to_string(i), t[i]...);
		}
	}

	template <class F, class... Types, class... Instances>
	void introspect_body(
		const std::tuple<Types...>* const,
		F f,
		Instances&&... t
	) {
		templates_detail::for_each_through_std_get(
			[f](auto num, auto&&... args) {
				f(std::to_string(num), std::forward<decltype(args)>(args)...);
			},
			std::index_sequence_for<Types...>{},
			std::forward<Instances>(t)...
		);
	}

	template <class F, class... Types, class... Instances>
	void introspect_body(
		const trivially_copyable_tuple<Types...>* const,
		F f,
		Instances&&... t
	) {
		templates_detail::for_each_through_std_get(
			[f](auto num, auto&&... args) {
				f(std::to_string(num), std::forward<decltype(args)>(args)...);
			},
			std::index_sequence_for<Types...>{},
			std::forward<Instances>(t)...
		);
	}
}

%xnamespace augs {
%x}