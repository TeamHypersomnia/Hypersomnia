#pragma once
#include <type_traits>

#include "generated/introspectors.h"

#include "augs/templates/traits/is_std_array.h"
#include "augs/templates/traits/container_traits.h"
#include "augs/templates/traits/introspection_traits.h"
#include "augs/templates/traits/enum_introspection_traits.h"
#include "augs/templates/recursive.h"
#include "augs/templates/traits/is_optional.h"
#include "augs/templates/traits/is_comparable.h"
#include "augs/templates/traits/is_tuple.h"

#include "augs/templates/introspect_declaration.h"

namespace augs {
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
		Instance& t,
		Instances&... tn
	) {
		using T = std::remove_reference_t<Instance>;
		static_assert(has_introspect_v<T>, "Recursion requested on type(s) without introspectors!");

		introspection_access::introspect_body(
			static_cast<std::decay_t<Instance>*>(nullptr), 
			std::forward<F>(callback), t, tn...
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
		Instance& t,
		Instances&... tn
	) {
		using T = std::remove_reference_t<Instance>;

		if constexpr(!is_introspective_leaf_v<T>) {
			introspection_access::introspect_body(
				static_cast<std::decay_t<Instance>*>(nullptr),
				std::forward<F>(callback), t, tn...
			);
		}
	}

	template <class A, class B>
	bool equal_or_introspective_equal(
		const A& a,
		const B& b
	) {
		if constexpr(is_optional_v<A>) {
			static_assert(is_optional_v<B>);

			if (a.has_value() != b.has_value()) {
				return false;
			}
			else if (a && b) {
				return equal_or_introspective_equal(*a, *b);
			}
			else {
				ensure(!a && !b);
				return true;
			}
		}
		else if constexpr(
			is_tuple_v<A>
			|| is_std_array_v<A>
		) {
			return introspective_equal(a, b);
		}
		else if constexpr(is_comparable_v<A, B>) {
			return a == b;
		}
		else {
			return introspective_equal(a, b);
		}
	}

	template <class A, class B>
	bool introspective_equal(const A& a, const B& b) {
		static_assert(has_introspect_v<A> && has_introspect_v<B>, "Comparison requested on type(s) without introspectors!");

		bool are_equal = true;

		introspect(
			[&are_equal](auto, const auto& aa, const auto& bb) {
				are_equal = are_equal && equal_or_introspective_equal(aa, bb);
			}, a, b
		);

		return are_equal;
	}

	template <class T>
	void recursive_clear(T& object) {
		introspect(recursive([](auto self, auto, auto& field) {
			using Field = std::decay_t<decltype(field)>;

			if constexpr(can_clear_v<Field>) {
				field.clear();
			}
			else {
				introspect_if_not_leaf(recursive(self), field);
			}
		}), object);
	}
}