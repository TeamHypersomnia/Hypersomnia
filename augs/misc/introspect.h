#pragma once
#include <type_traits>
#include <xtr1common>
#include "augs/templates/maybe_const.h"
#include "augs/templates/is_traits.h"

namespace augs {
	template <class T, class F>
	void introspect(
		T& t,
		F f
	) {
		introspect<std::is_const_v<T>>(t, f);
	}

	template <
		template <class A> class call_valid_predicate,
		class MemberType,
		class = void
	>
	struct recursive_introspector;

	template <
		template <class A> class call_valid_predicate,
		class MemberType
	>	
	struct recursive_introspector<
		call_valid_predicate,
		MemberType,
		std::enable_if_t<call_valid_predicate<MemberType>::value>
	> {
		template <class F, class... Args>
		void operator()(
			F callback,
			MemberType& member,
			Args&&... args
		) {
			callback(member, std::forward<Args>(args)...);
		}
	};

	template <
		template <class A> class call_valid_predicate,
		class MemberType
	>
	struct recursive_introspector<
		call_valid_predicate,
		MemberType,
		std::enable_if_t<!call_valid_predicate<MemberType>::value && is_introspective_leaf_v<MemberType>>
	> {
		template <class F, class... Args>
		void operator()(
			F callback,
			MemberType& member,
			Args&&... args
		) {

		}
	};

	template <
		template <class A> class call_valid_predicate,
		class MemberType
	>
	struct recursive_introspector<
		call_valid_predicate,
		MemberType,
		std::enable_if_t<!call_valid_predicate<MemberType>::value && !is_introspective_leaf_v<MemberType>>
	> {
		template <class F, class... Args>
		void operator()(
			F callback,
			MemberType& member,
			Args&&... args
		) {
			static_assert(has_introspects_v<MemberType>, "Found a non-fundamental type without an introspector, on whom the callback is invalid.");

			introspect_recursive<call_valid_predicate>(member, callback);
		}
	};

	template <
		template <class A> class call_valid_predicate,
		class MemberType,
		class F
	>
	void introspect_recursive(
		MemberType& t,
		F member_callback
	) {
		introspect<std::is_const_v<MemberType>>(
			t,
			[&](auto& member, auto... args) {
				recursive_introspector<
					call_valid_predicate,
					std::decay_t<decltype(member)>
				>()(member_callback, member, args...);
			}
		);
	}
}

struct true_returner {
	template <class... Types>
	bool operator()(Types...) const {
		return true;
	}
};

template <class T, bool C, class = void>
struct has_introspect {
	static constexpr bool value = false;
};

template <class T, bool C>
struct has_introspect<
	T, 
	C,
	decltype(
		augs::introspect<C>(
			std::declval<
				std::conditional_t<C, const T, T>
			>(),
			true_returner()
		), 
		void()
	)
> {
	static constexpr bool value = true;
};

template <class T, bool C>
constexpr bool has_introspect_v = has_introspect<T, C>::value;

template <class T>
struct has_introspects {
	static constexpr bool value = has_introspect_v<T, false> && has_introspect_v<T, true>;
}; 

template <class T>
constexpr bool has_introspects_v = has_introspects<T>::value;