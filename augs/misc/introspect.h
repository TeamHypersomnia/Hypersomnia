#pragma once
#include <type_traits>
#include <xtr1common>
#include <array>

#include "augs/templates/maybe_const.h"
#include "augs/templates/is_traits.h"

namespace augs {
	template <bool C, class F, class ElemType, size_t count>
	void introspect_body(
		maybe_const_ref_t<C, std::array<ElemType, count>> t,
		F f,
		const std::array<ElemType, count>* const
	) {
		for (size_t i = 0; i < count; ++i) {
			f(*(t.data() + i), std::to_string(i));
		}
	}

	template <class T, class F>
	void introspect(
		T& t,
		F f
	) {
		introspect_body<std::is_const_v<T>>(t, f, &t);
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
		class T,
		class F
	>
	void introspect_recursive(
		T& t,
		F member_callback
	) {
		introspect(
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
		augs::introspect_body<C>(
			std::declval<
				maybe_const_ref_t<C, T>
			>(),
			true_returner(),
			maybe_const_ptr_t<C, T>()
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