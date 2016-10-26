#pragma once
#include "augs/templates.h"
#include "augs/ensure.h"

namespace augs {
	template<class... Types>
	class trivial_variant {
		unsigned current_type = sizeof...(Types);
		typename std::aligned_union<0, Types...>::type _s;

		static_assert(are_types_memcpy_safe<Types...>::value, "Types must be memcpy-safe!");

		template<class L, class Head>
		decltype(auto) call_unroll(L f) {
			const bool polymorphic_call_target_found = index_in_pack<Head, Types...>::value == current_type;
			ensure(polymorphic_call_target_found);
			return f(get<Head>());
		}

		template<class L, class Head, class... Tail>
		decltype(auto) call_unroll(L f) {
			if (index_in_pack<Head, Types...>::value == current_type) {
				return f(get<Head>());
			}
			else {
				return call_unroll<L, Tail...>(f);
			}
		}

		//template<class L>
		//decltype(auto) call_or_zero_unroll(L f) {
		//	return static_cast<decltype(f())>(0);
		//}
		//
		//template<class L, class Head, class Tail...>
		//decltype(auto) call_or_zero_unroll(L f) {
		//	if (index_in_pack<Head, Types...>::value == current_type) {
		//		return f(get<Head>());
		//	}
		//	else {
		//		return call_unroll<L, Tail...>(f);
		//	}
		//}

	public:
		trivial_variant() {
			std::memset(&_s, 0, sizeof(_s));
		}

		bool is_set() const {
			return current_type != sizeof...(Types);
		}

		template<class T>
		void set(const T& t) {
			std::memcpy(_s, &t, sizeof(T));
			current_type = index_in_pack<T, Types...>::value;
		}

		template<class T>
		T& get() {
			const bool polymorphic_get_target_found = index_in_pack<T, Types...>::value == current_type;
			ensure(polymorphic_get_target_found);
			return *reinterpret_cast<T*>(_s);
		}

		template<class T>
		const T& get() const {
			const bool polymorphic_get_target_found = index_in_pack<T, Types...>::value == current_type;
			ensure(polymorphic_get_target_found);
			return *reinterpret_cast<const T*>(_s);
		}

		template<class L>
		decltype(auto) call(L polymorphic_call) {
			return call_unroll<L, Types...>(polymorphic_call);
		}
		
		//template<class L>
		//decltype(auto) call_or_zero(L polymorphic_call) {
		//	return call_or_zero_unroll<L, Types...>(polymorphic_call);
		//}
	};
}