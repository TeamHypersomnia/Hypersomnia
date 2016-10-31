#pragma once
#include "augs/templates.h"
#include "augs/ensure.h"

namespace augs {
	namespace detail {
		// TEMPLATE CLASS aligned_union
		template<size_t... _Vals>
		struct _maximum;

		template<>
		struct _maximum<>
		{	// maximum of nothing is 0
			static constexpr size_t value = 0;
		};

		template<size_t _Val>
		struct _maximum<_Val>
		{	// maximum of _Val is _Val
			static constexpr size_t value = _Val;
		};

		template<size_t _First,
			size_t _Second,
			size_t... _Rest>
			struct _maximum<_First, _Second, _Rest...>
			: _maximum<(_First < _Second ? _Second : _First), _Rest...>
		{	// find maximum value in _First, _Second, _Rest...
		};

	}

	template<class... Types>
	class trivial_variant {
		unsigned current_type = sizeof...(Types);
		char buf[detail::_maximum<0, sizeof(Types)...>::value];
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

	public:
		template<class T>
		static void assert_correct_type() {
			static_assert(pack_contains_type<T, Types...>::value, "trivial_variant does not contain the specified type!");
		}

		typedef std::tuple<Types...> types_tuple;

		trivial_variant() {
			std::memset(buf, 0, sizeof(buf));
		}

		template <class T>
		trivial_variant(const T& obj) : trivial_variant() {
			set(obj);
		}

		void unset() {
			current_type = sizeof...(Types);
		}

		bool is_set() const {
			return current_type != sizeof...(Types);
		}

		template<class T_convertible>
		void set(const T_convertible& t) {
			typedef find_convertible_type<T_convertible, Types...> T;

			std::memcpy(buf, &t, sizeof(T));
			current_type = index_in_pack<T, Types...>::value;
		}

		template<class T_convertible>
		bool operator==(const T_convertible& b) const {
			typedef find_convertible_type<T_convertible, Types...> T;
			return is<T>() && get<T>() == b;
		}

		template<class T>
		bool is() const {
			assert_correct_type<T>();
			return index_in_pack<T, Types...>::value == current_type;
		}

		template<class T>
		T& get() {
			assert_correct_type<T>();
			ensure(is<T>());
			return *reinterpret_cast<T*>(buf);
		}

		template<class T>
		const T& get() const {
			assert_correct_type<T>();
			ensure(is<T>());
			return *reinterpret_cast<const T*>(buf);
		}

		template<class T>
		T* find() {
			assert_correct_type<T>();
			return is<T>() ? reinterpret_cast<T*>(buf) : nullptr;
		}

		template<class T>
		const T* find() const {
			assert_correct_type<T>();
			return is<T>() ? reinterpret_cast<T*>(buf) : nullptr;
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