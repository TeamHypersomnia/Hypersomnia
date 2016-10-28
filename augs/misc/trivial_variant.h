#pragma once
#include "augs/templates.h"
#include "augs/ensure.h"

namespace augs {
	template<class... Types>
	class trivial_variant {
		unsigned current_type = sizeof...(Types);
		typename std::aligned_union<0, Types...>::type _s;

		static_assert(are_types_memcpy_safe<Types...>::value, "Types must be memcpy-safe!");

		template<class L>
		decltype(auto) call_unroll(L f) {
			const bool failed_to_find_polymorphic_candidate = true;
			ensure(!failed_to_find_polymorphic_candidate);

			return f(get<nth_type_in_t<0, Types...>>());
		}

		template<class L>
		decltype(auto) call_unroll_const(L f) const {
			const bool failed_to_find_polymorphic_candidate = true;
			ensure(!failed_to_find_polymorphic_candidate);

			return f(get<nth_type_in_t<0, Types...>>());
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

		template<class L, class Head, class... Tail>
		decltype(auto) call_unroll_const(L f) const {
			if (index_in_pack<Head, Types...>::value == current_type) {
				return f(get<Head>());
			}
			else {
				return call_unroll_const<L, Tail...>(f);
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
		typedef std::tuple<Types...> types_tuple;

		trivial_variant() {
			std::memset(&_s, 0, sizeof(_s));
		}

		bool is_set() const {
			return current_type != sizeof...(Types);
		}

		template<class T>
		void set(const T& t) {
			std::memcpy(&_s, &t, sizeof(T));
			current_type = index_in_pack<T, Types...>::value;
		}

		template<class T>
		bool is() const {
			return index_in_pack<T, Types...>::value == current_type;
		}

		template<class T>
		T& get() {
			ensure(is<T>());
			return *reinterpret_cast<T*>(&_s);
		}

		template<class T>
		const T& get() const {
			ensure(is<T>());
			return *reinterpret_cast<const T*>(&_s);
		}

		template<class T>
		T* find() {
			return is<T>() ? reinterpret_cast<T*>(&_s) : nullptr;
		}

		template<class T>
		const T* find() const {
			return is<T>() ? reinterpret_cast<T*>(&_s) : nullptr;
		}

		template<class L>
		decltype(auto) call(L generic_call) {
			return call_unroll<L, Types...>(generic_call);
		}

		template<class L>
		decltype(auto) call(L generic_call) const {
			return call_unroll_const<L, Types...>(generic_call);
		}

		bool operator==(const trivial_variant& b) const {
			return current_type == b.current_type 
				&& 
				call([&](const auto& resolved_a) {
					return resolved_a == b.get<std::decay_t<decltype(resolved_a)>>();
				});
		}

		//template<class L>
		//decltype(auto) call_or_zero(L generic_call) {
		//	return call_or_zero_unroll<L, Types...>(generic_call);
		//}
	};
}

namespace std {
	template <class... Types>
	struct hash<augs::trivial_variant<Types...>> {
		std::size_t operator()(const augs::trivial_variant<Types...>& k) const {
			return k.call([](auto resolved) {
				return std::hash<decltype(resolved)>()(resolved);
			});
		}
	};
}