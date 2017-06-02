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
		augs::introspection_access::introspect_body(
			static_cast<std::decay_t<Instance>*>(nullptr), 
			std::forward<F>(callback),
			std::forward<Instance>(t), 
			std::forward<Instances>(tn)...
		);
	}

	/*
		Because the introspected members will always be passed to the callback as some kind of reference,
		it makes sense to remove the references from the arguments to predicates 
		to simplify the implementation of "should recurse" and "call valid" predicates.
	
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

	/*
		Explanation of call_valid_predicate and should_recurse_predicate:

		if should_recurse_predicate returns true on a given set of types
			invoke the callback upon the types
		if should_recurse_predicate returns true on a given set of types and none of them is an introspective leaf
			recurse (static_assert ensures that introspectors exist at this point)

		I pass the arguments for the lambda via constexpr_if's Args&& right under the lambda itself, 
		instead of letting them being captured by [&] - compiler is faulty when it comes to capturing parameter packs.
	*/

	template <
		template <class...> class call_valid_predicate,
		template <class...> class should_recurse_predicate,
		bool stop_recursion_if_valid,
		unsigned current_depth,
		class F,
		class G,
		class H,
		class Instance,
		class... Instances
	>
	void introspect_recursive_with_prologues(
		F&& member_callback,
		G&& recursion_prologue,
		H&& recursion_epilogue,
		Instance&& introspected_instance,
		Instances&&... introspected_instances
	) {
		introspect(
			[&](
				auto&& label, 
				auto&&... args
			) {
				if constexpr(eval<call_valid_predicate, decltype(args)...>()) {
					member_callback(
						std::forward<decltype(label)>(label),
						std::forward<decltype(args)>(args)...
					);
				}

				if constexpr(
					eval<should_recurse_predicate, decltype(args)...>()
					&& eval<at_least_one_is_not_introspective_leaf, decltype(args)...>()
					&& !(eval<call_valid_predicate, decltype(args)...>() && stop_recursion_if_valid)
				) {
					static_assert(eval<have_introspects, decltype(args)...>(), "Recursion requested on type(s) without introspectors!");
					
					recursion_prologue(
						current_depth,
						std::forward<decltype(label)>(label),
						std::forward<decltype(args)>(args)...
					);

					introspect_recursive_with_prologues <
						call_valid_predicate,
						should_recurse_predicate,
						stop_recursion_if_valid,
						current_depth + 1u
					> (
						std::forward<F>(member_callback),
						std::forward<G>(recursion_prologue),
						std::forward<H>(recursion_epilogue),
						std::forward<decltype(args)>(args)...
					);

					recursion_epilogue(
						current_depth,
						std::forward<decltype(label)>(label),
						std::forward<decltype(args)>(args)...
					);
				}
			},
			std::forward<Instance>(introspected_instance),
			std::forward<Instances>(introspected_instances)...
		);
	}

	/*
		An overload for a single argument allows us to pass an object that may have a variable number of members. 
		In particular, it allows us to invoke the callback upon every object owned by variable-length container members.

		This would not be possible with two or more instances, because if a container of one instance has more members
		than the corresponding container of the second instance, there would later be no second argument to supply for the callback.
	*/

	template <
		template <class...> class call_valid_predicate,
		template <class...> class should_recurse_predicate,
		bool stop_recursion_if_valid,
		unsigned current_depth,
		class F,
		class G,
		class H,
		class Instance
	>
	void introspect_recursive_with_prologues(
		F&& member_callback,
		G&& recursion_prologue,
		H&& recursion_epilogue,
		Instance&& introspected_instance
	) {
		introspect(
			[&](
				auto&& label, 
				auto&& arg
			) {
				if constexpr(eval<call_valid_predicate, decltype(arg)>()) {
					member_callback(
						std::forward<decltype(label)>(label),
						std::forward<decltype(arg)>(arg)
					);
				}

				if constexpr(
					eval<should_recurse_predicate, decltype(arg)>()
					&& eval<at_least_one_is_not_introspective_leaf, decltype(arg)>()
					&& !(eval<call_valid_predicate, decltype(arg)>() && stop_recursion_if_valid)
				) {
					using checked_type = std::remove_reference_t<decltype(arg)>;

					static_assert(has_introspect_v<checked_type> || is_variable_size_container_v<checked_type>, 
						"Recursion requested on type without introspectors, that is not an iteratable container!"
					);
					
					recursion_prologue(
						current_depth,
						std::forward<decltype(label)>(label),
						std::forward<decltype(arg)>(arg)
					);
					
					/* 
						If the container is not introspectable, iterate through all members,
						otherwise use the introspector defined for this container 
					*/

					if constexpr(!has_introspect_v<checked_type> && is_variable_size_container_v<checked_type>){
						for (auto& val : arg) {
							introspect_recursive_with_prologues <
								call_valid_predicate,
								should_recurse_predicate,
								stop_recursion_if_valid,
								current_depth + 1u
							> (
								std::forward<F>(member_callback),
								std::forward<G>(recursion_prologue),
								std::forward<H>(recursion_epilogue),
								val
							);
						}
					}
					else {
						introspect_recursive_with_prologues <
							call_valid_predicate,
							should_recurse_predicate,
							stop_recursion_if_valid,
							current_depth + 1u
						>(
							std::forward<F>(member_callback),
							std::forward<G>(recursion_prologue),
							std::forward<H>(recursion_epilogue),
							std::forward<decltype(arg)>(arg)
						);
					}

					recursion_epilogue(
						current_depth,
						std::forward<decltype(label)>(label),
						std::forward<decltype(arg)>(arg)
					);
				}
			},
			std::forward<Instance>(introspected_instance)
		);
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
		introspect_recursive_with_prologues<
			call_valid_predicate,
			should_recurse_predicate,
			stop_recursion_if_valid,
			0u
		>(
			std::forward<F>(member_callback),
			no_prologue(),
			no_epilogue(),
			std::forward<Instances>(introspected_instances)...
		);
	}
}