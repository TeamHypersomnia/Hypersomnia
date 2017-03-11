#pragma once
#include <type_traits>
#include <xtr1common>
#include <array>

#include "augs/templates/maybe_const.h"
#include "augs/templates/is_traits.h"

template <class T>
struct exclude_no_type {
	static constexpr bool value = false;
};

namespace augs {
	template <class F, class ElemType, size_t count, class... MemberInstances>
	void introspect_body(
		const std::array<ElemType, count>* const,
		F f,
		MemberInstances&&... t
	) {
		for (size_t i = 0; i < count; ++i) {
			f(std::to_string(i), t[i]...);
		}
	}

	template <
		class F, 
		class Instance, 
		class... Instances
	>
	void introspect(
		F callback,
		Instance&& t,
		Instances&&... tn
	) {
		introspect_body(
			static_cast<std::decay_t<Instance>*>(nullptr), 
			callback, 
			std::forward<Instance>(t), 
			std::forward<Instances>(tn)...
		);
	}

	template <
		template <class A> class call_valid_predicate,
		template <class B> class exclude_type_predicate,
		class MemberType,
		class = void
	>
	struct recursive_introspector;

	template <
		template <class A> class call_valid_predicate,
		template <class B> class exclude_type_predicate,
		class MemberType
	>	
	struct recursive_introspector<
		call_valid_predicate,
		exclude_type_predicate,
		MemberType,
		std::enable_if_t<
			call_valid_predicate<MemberType>::value 
			&& !exclude_type_predicate<MemberType>::value
		>
	> {
		template <class F, class L, class... Args>
		void operator()(
			F callback,
			const L& label,
			Args&&... args
		) {
			callback(
				label,
				std::forward<Args>(args)...
			);
		}
	};
	

	template <
		template <class A> class call_valid_predicate,
		template <class B> class exclude_type_predicate,
		class MemberType
	>
	struct recursive_introspector<
		call_valid_predicate,
		exclude_type_predicate,
		MemberType,
		std::enable_if_t<
			!call_valid_predicate<MemberType>::value 
			&& !is_introspective_leaf_v<MemberType>
			&& !exclude_type_predicate<MemberType>::value
		>
	> {
		template <class F, class L, class... Args>
		void operator()(
			F callback,
			const L& label,
			Args&&... args
		) {
			static_assert(has_introspects_v<MemberType>, "Found a non-fundamental type without an introspector, on whom the callback is invalid.");

			introspect_recursive<
				call_valid_predicate,
				exclude_type_predicate
			>(
				callback,
				std::forward<Args>(args)...
			);
		}
	};

	template <
		template <class A> class call_valid_predicate,
		template <class B> class exclude_type_predicate,
		class MemberType
	>
	struct recursive_introspector<
		call_valid_predicate,
		exclude_type_predicate,
		MemberType,
		std::enable_if_t<
			(
				!call_valid_predicate<MemberType>::value 
				&& is_introspective_leaf_v<MemberType>
			) 
			|| exclude_type_predicate<MemberType>::value
		>
	> {
		template <class F, class... Args>
		void operator()(
			F callback,
			Args&&... args
		) {

		}
	};

	template <
		template <class A> class call_valid_predicate,
		template <class B> class exclude_type_predicate,
		class F,
		class... Instances
	>
	void introspect_recursive(
		F member_callback,
		Instances&&... tn
	) {
		introspect(
			[&](
				const auto& label, 
				auto& first_instance, 
				auto&&... instances
			) {
				recursive_introspector<
					call_valid_predicate,
					exclude_type_predicate,
					std::decay_t<decltype(first_instance)>
				>()(
					member_callback,
					label,
					first_instance,
					std::forward<decltype(instances)>(instances)...
				);
			},
			std::forward<Instances>(tn)...
		);
	}
}

struct true_returner {
	template <class... Types>
	bool operator()(Types...) const {
		return true;
	}
};

template <class T, class = void>
struct has_introspect {
	static constexpr bool value = false;
};

template <class T>
struct has_introspect<
	T, 
	decltype(
		augs::introspect_body(
			static_cast<T*>(nullptr),
			true_returner(),
			std::declval<T>()
		), 
		void()
	)
> {
	static constexpr bool value = true;
};

template <class T>
constexpr bool has_introspects_v = has_introspect<T>::value;