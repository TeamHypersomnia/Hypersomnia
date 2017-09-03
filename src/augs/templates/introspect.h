#pragma once
#include <type_traits>
#include "augs/templates/container_templates.h"
#include "augs/templates/introspection_traits.h"
#include "augs/templates/enum_introspect.h"
#include "augs/templates/recursive.h"

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

	template <class A, class B>
	bool introspective_compare(
		const A& a,
		const B& b
	) {
		bool are_same = true;

		augs::introspect(
			augs::recursive([&are_same](
				auto&& self,
				const auto label,
				const auto& aa, 
				const auto& bb
			) {
				using A = std::decay_t<decltype(aa)>;
				using B = std::decay_t<decltype(bb)>;

				if constexpr(is_comparable_v<A, B>) {
					are_same = are_same && aa == bb;
				}
				else {
					augs::introspect(augs::recursive(self), aa, bb);
				}
			}),
			a,
			b
		);
		
		return are_same;
	}
}