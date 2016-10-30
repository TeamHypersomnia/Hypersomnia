#pragma once
#include "augs/templates.h"
#include "augs/ensure.h"

namespace augs {
	namespace detail {
		template <class derived, class... Types>
		struct unrolled_members;

		template <class derived, class Head>
		struct unrolled_members<derived, Head> {
			void set(const Head& h) {
				auto* self = static_cast<derived*>(this);
				self->set_internal(h);
			}

			bool operator==(const Head& b) const {
				const auto* self = static_cast<const derived*>(this);
				return self->compare_internal(b);
			}
		};

		template <class derived, class Head, class... Tail>
		struct unrolled_members<derived, Head, Tail...> : unrolled_members<derived, Head>, unrolled_members<derived, Tail...> {
			using unrolled_members<derived, Tail...>::set;
			using unrolled_members<derived, Head>::set;			
			using unrolled_members<derived, Tail...>::operator==;
			using unrolled_members<derived, Head>::operator==;
		};
	}

	template<class... Types>
	class trivial_variant : public detail::unrolled_members<trivial_variant<Types...>, Types...> {
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

		template<class T>
		void set_internal(const T& t) {
			assert_correct_type<T>();
			std::memcpy(&_s, &t, sizeof(T));
			current_type = index_in_pack<T, Types...>::value;
		}

		template<class T>
		bool compare_internal(const T& b) const {
			assert_correct_type<T>();
			return is<T>() && get<T>() == b;
		}

		template<class, class...>
		friend struct detail::unrolled_members;

	public:
		template<class T>
		static void assert_correct_type() {
			static_assert(pack_contains_type<T, Types...>::value, "trivial_variant does not contain the specified type!");
		}

		typedef std::tuple<Types...> types_tuple;

		trivial_variant() {
			std::memset(&_s, 0, sizeof(_s));
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

		template<class T>
		bool is() const {
			assert_correct_type<T>();
			return index_in_pack<T, Types...>::value == current_type;
		}

		template<class T>
		T& get() {
			assert_correct_type<T>();
			ensure(is<T>());
			return *reinterpret_cast<T*>(&_s);
		}

		template<class T>
		const T& get() const {
			assert_correct_type<T>();
			ensure(is<T>());
			return *reinterpret_cast<const T*>(&_s);
		}

		template<class T>
		T* find() {
			assert_correct_type<T>();
			return is<T>() ? reinterpret_cast<T*>(&_s) : nullptr;
		}

		template<class T>
		const T* find() const {
			assert_correct_type<T>();
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