#pragma once
#include <type_traits>
#include "augs/templates/introspection_traits.h"

namespace augs {
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
		introspect_body(
			static_cast<std::decay_t<Instance>*>(nullptr), 
			std::forward<F>(callback),
			std::forward<Instance>(t), 
			std::forward<Instances>(tn)...
		);
	}

	/*
		Explanation of conditional specialization:

		if call on given types is valid
			call
		if should recurse with given types and types are not introspective leaves
			recurse (static_asserts that introspectors exist at this point)
	*/

	template <
		bool flag,
		class F,
		class... Args
	>
	void conditional_call(
		F&& callback,
		Args&&... args,
		std::enable_if_t<flag>* = nullptr
	) {
		callback(std::forward<Args>(args)...);
	}

	template <
		bool flag,
		class F,
		class... Args
	>
	void conditional_call(
		F&& callback,
		Args&&...,
		std::enable_if_t<!flag>* = nullptr
	) {

	}

	/*
		Because the introspected members will always be passed to the callback as some kind of reference,
		it makes sense to remove them from the argument to predicates to simplify the implementation of "should recurse" and "call valid" predicates.
	
		It is a separate function also for the reason that the compiler has troubles unpacking the parameter packs in lambdas,
		when the pattern is more complex.
	*/

	template <
		template <class...> class pred,
		class... Args
	>
	inline constexpr bool eval() {
		return pred<std::remove_reference_t<Args>...>::value;
	}

	template <
		template <class...> class call_valid_predicate,
		template <class...> class should_recurse_predicate,
		bool stop_recursion_if_valid,
		class F,
		class... Instances
	>
	void introspect_recursive(
		F&& member_callback,
		Instances&&... introspected_instances
	) {
		introspect(
			[&](
				auto&& label, 
				auto&&... args
			) {
				conditional_call<
					eval<call_valid_predicate, decltype(args)...>()
				> (
					[&](auto...) {
						member_callback(
							std::forward<decltype(label)>(label),
							std::forward<decltype(args)>(args)...
						);
					}
				);

				conditional_call<
					eval<should_recurse_predicate, decltype(args)...>()
					&& eval<is_any_not_an_introspective_leaf, decltype(args)...>()
					&& !(eval<call_valid_predicate, decltype(args)...>() && stop_recursion_if_valid)
				> (
					[&](auto...) {
						static_assert(eval<have_introspects, decltype(args)...>(), "Recursion requested on type(s) without introspectors!");
						
						introspect_recursive <
							call_valid_predicate,
							should_recurse_predicate,
							stop_recursion_if_valid
						> (
							std::forward<F>(member_callback),
							std::forward<decltype(args)>(args)...
						);
					}
				);
			},
			std::forward<Instances>(introspected_instances)...
		);
	}
}