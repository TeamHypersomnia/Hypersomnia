#pragma once
#include "augs/templates.h"
#include "augs/ensure.h"

namespace augs {
	template<class... Types>
	class trivial_variant {
		unsigned current_type = sizeof...(Types);
		char _s[std::max({ sizeof(Types)... })];

		static_assert(are_types_memcpy_safe<Types...>::value, "Types must be memcpy-safe!");

		template<class L, class Head>
		decltype(auto) call_unroll(L f) {
			ensure(index_in_pack<Head, Types...>::value == current_type && "Polymorphic call on a null location!");
			return f(get<Head>());
		}

		template<class L, class Head, class Tail...>
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
			ensure(index_in_pack<T, Types...>::value == current_type);
			return *reinterpret_cast<T*>(_s);
		}

		template<class T>
		const T& get() const {
			ensure(index_in_pack<T, Types...>::value == current_type);
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