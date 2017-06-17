#pragma once
#include <type_traits>
#include "augs/templates/introspection_traits.h"

namespace augs {
	struct introspection_access;

	/*
		Simple introspection with just one level of depth.
		Will invoke a callback upon every top-level field of a struct.
	*/

	template <
		class F, 
		class Instance, 
		class... Instances
	>
	void introspect(
		F&& callback,
		Instance&& t,
		Instances&&... tn
	) {
		using T = std::remove_reference_t<Instance>;
		static_assert(has_introspect_v<T>, "Recursion requested on type(s) without introspectors!");

		augs::introspection_access::introspect_body(
			static_cast<std::decay_t<Instance>*>(nullptr), 
			std::forward<F>(callback),
			std::forward<Instance>(t), 
			std::forward<Instances>(tn)...
		);
	}
	/*
		Simple introspection with just one level of depth.
		Will invoke a callback upon every top-level field of a struct.
		Will not invoke the callback if all the types are introspective leaves.
		If a type is not an introspective leaf, but does not possess an introspector, a compilation error will be raised.
	*/

	template <
		class F, 
		class Instance, 
		class... Instances
	>
	void introspect_if_not_leaf(
		F&& callback,
		Instance&& t,
		Instances&&... tn
	) {
		using T = std::remove_reference_t<Instance>;

		if constexpr(!is_introspective_leaf_v<T>) {
			augs::introspection_access::introspect_body(
				static_cast<std::decay_t<Instance>*>(nullptr),
				std::forward<F>(callback),
				std::forward<Instance>(t),
				std::forward<Instances>(tn)...
			);
		}
	}

	template <class Enum, class F>
	void for_each_enum(F callback) {
		augs::enum_to_args_impl(
			Enum(),
			[callback](const auto... all_enums) {
				for (const auto _enum : { all_enums }) {
					callback(_enum);
				}
			}
		);
	}
}