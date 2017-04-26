#pragma once
#include <tuple>
#include <array>

#include "augs/misc/trivially_copyable_tuple.h"
#include "augs/templates/for_each_in_types.h"

#include "game/transcendental/entity_id_declaration.h"

#define FIELD(x) f(#x, _t_.x...)

// Forward declarations

%xnamespace augs {
	struct introspection_access {
		/* Hand-written introspectors that do not fit into the standard schema begin here */

		template <class F, class ElemType, size_t count, class... Instances>
		static void introspect_body(
			const std::array<ElemType, count>* const,
			F f,
			Instances&&... t
		) {
			for (size_t i = 0; i < count; ++i) {
				f(std::to_string(i), t[i]...);
			}
		}

		template <class F, class... Types, class... Instances>
		static void introspect_body(
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
		static void introspect_body(
			const augs::trivially_copyable_tuple<Types...>* const,
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

		/* Generated introspectors begin here */

%x	};
}