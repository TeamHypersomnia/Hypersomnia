#pragma once
#include <type_traits>

#include "generated/introspectors.h"

#include "augs/templates/for_each_type.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/introspection_traits.h"

#include "augs/templates/introspect_declaration.h"

namespace augs {
	template <class F, class Instance, class... Instances>
	void introspect_body(
		F&& callback,
		Instance& t,
		Instances&... tn
	) {
		using T = std::remove_reference_t<Instance>;
		static_assert(has_introspect_body_v<T>, "No introspector found for the type.");

		introspection_access::introspect_body(
			static_cast<remove_cref<Instance>*>(nullptr), 
			std::forward<F>(callback), t, tn...
		);
	}

	/*
		Simple introspection with just one level of depth.
		Will invoke a callback upon every top-level field of a struct,
		and also on all fields of base classes specified with either of these two:

		using introspect_base = ...;
		using introspect_bases = type_list<...>;
	*/

	template <class F, class Instance, class... Instances>
	void introspect(
		F callback,
		Instance& t,
		Instances&... tn
	) {
		using T = std::remove_reference_t<Instance>;
		static constexpr bool C = std::is_const_v<T>;

		if constexpr(has_introspect_base_v<T>) {
			introspect(
				callback, 
				static_cast<maybe_const_ref_t<C, typename T::introspect_base>>(t),
				tn...
			);
		}
		else if constexpr(has_introspect_bases_v<T>) {
			for_each_type_in_list<typename T::introspect_bases>([&](const auto& b){
				introspect(
					callback, 
					static_cast<maybe_const_ref_t<C, remove_cref<decltype(b)>>>(t),
					tn...
				);
			});
		}

		static_assert(
			(!has_introspect_bases_v<T> && !has_introspect_bases_v<T>) || (has_introspect_bases_v<T> != has_introspect_base_v<T>),
		   	"Please choose only one way to specify introspected bases."
		);

		if constexpr(has_introspect_body_v<T>) {
			introspect_body(callback, t, tn...);
		}
		else {
			static_assert(
				has_introspect_bases_v<T> || has_introspect_base_v<T>,
			   	"No introspector found for the types, and no introspected bases were specified."
			);
		}
	}
}