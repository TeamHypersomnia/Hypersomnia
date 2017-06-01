#pragma once
#include "augs/templates/type_matching_and_indexing.h"
#include "augs/templates/memcpy_safety.h"
#include "augs/templates/dynamic_dispatch.h"
#include "augs/templates/constexpr_arithmetic.h"
#include "augs/ensure.h"

namespace augs {
	template <class... Types>
	class trivial_variant {
		type_in_list_id<trivial_variant<Types...>> current_type;
		char buf[constexpr_max_v<std::size_t, 0, sizeof(Types)...>];
		static_assert(are_types_memcpy_safe_v<Types...>, "Types must be memcpy-safe!");

	public:
		trivial_variant() {
			std::memset(buf, 0, sizeof(buf));
		}

		template<class T_convertible>
		trivial_variant(const T_convertible& obj) : trivial_variant() {
			set(obj);
		}

		void unset() {
			current_type.unset();
		}

		bool is_set() const {
			return current_type.is_set();
		}

		template<class T_convertible>
		void set(const T_convertible& t) {
			typedef find_convertible_type_in_t<T_convertible, Types...> T;

			const auto converted = T(t);
			std::memcpy(buf, &converted, sizeof(T));
			current_type.set<T>();
		}

		template<class T_convertible>
		bool operator==(const T_convertible& b) const {
			typedef find_convertible_type_in_t<T_convertible, Types...> T;
			return is<T>() && get<T>() == b;
		}

		template <class T>
		bool is() const {
			return current_type.is<T>();
		}

		template <std::size_t I>
		auto& get() {
			return get<nth_type_in_t<I, Types...>>();
		}

		template <std::size_t I>
		const auto& get() const {
			return get<nth_type_in_t<I, Types...>>();
		}

		template <class T>
		T& get() {
			ensure(is<T>());
			return *reinterpret_cast<T*>(buf);
		}

		template <class T>
		const T& get() const {
			ensure(is<T>());
			return *reinterpret_cast<const T*>(buf);
		}

		template<class T>
		T* find() {
			return is<T>() ? reinterpret_cast<T*>(buf) : nullptr;
		}

		template<class T>
		const T* find() const {
			return is<T>() ? reinterpret_cast<T*>(buf) : nullptr;
		}

		template<class L>
		decltype(auto) call(L&& generic_call) {
			return dynamic_dispatch(*this, current_type, std::forward<L>(generic_call));
		}

		template<class L>
		decltype(auto) call(L&& generic_call) const {
			return dynamic_dispatch(*this, current_type, std::forward<L>(generic_call));
		}

		auto get_current_type_id() const {
			return current_type;
		}

		bool operator==(const trivial_variant& b) const {
			return 
				current_type == b.current_type 
				&& call(
					[&](const auto& resolved_a) {
						return resolved_a == b.get<std::decay_t<decltype(resolved_a)>>();
					}
				)
			;
		}
	};
}

namespace std {
	template<std::size_t I, class... Types>
	decltype(auto) get(augs::trivial_variant<Types...>& t) {
		return t.get<I>();
	}

	template<std::size_t I, class... Types>
	decltype(auto) get(const augs::trivial_variant<Types...>& t) {
		return t.get<I>();
	}

	template<class T, class... Types>
	decltype(auto) get(augs::trivial_variant<Types...>& t) {
		return t.get<T>();
	}

	template<class T, class... Types>
	decltype(auto) get(const augs::trivial_variant<Types...>& t) {
		return t.get<T>();
	}

	template <class... Types>
	struct hash<augs::trivial_variant<Types...>> {
		std::size_t operator()(const augs::trivial_variant<Types...>& k) const {
			return k.call([&k](const auto& resolved) {
				return augs::simple_two_hash(k.get_current_type_id(), resolved);
			});
		}
	};
}