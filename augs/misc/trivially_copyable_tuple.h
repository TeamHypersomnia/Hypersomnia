#pragma once
#include <tuple>
#include "augs/templates/memcpy_safety.h"

namespace augs {
	template <class... Types>
	class trivially_copyable_tuple {
		typedef std::tuple<Types...> tuple_type;

		alignas(tuple_type) char buf[sizeof(tuple_type)];
	public:
		static_assert(are_types_memcpy_safe_v<Types...>, "One of the tuple types is not trivially copyable!");

		template <class... Args>
		trivially_copyable_tuple(Args&&... args) {
			// zero-initialize the memory so that delta encoding does not see the padding bytes as different
			std::memset(this, sizeof(*this), 0);

			new (buf) tuple_type(std::forward<Args>(args)...);
		}

		tuple_type& get_tuple() {
			return *reinterpret_cast<tuple_type*>(buf);
		}

		const tuple_type& get_tuple() const {
			return *reinterpret_cast<const tuple_type*>(buf);
		}
	};
}

namespace std {
	template<int idx, class... Types>
	decltype(auto) get(augs::trivially_copyable_tuple<Types...>& t) {
		return std::get<idx>(t.get_tuple());
	}

	template<int idx, class... Types>
	decltype(auto) get(const augs::trivially_copyable_tuple<Types...>& t) {
		return std::get<idx>(t.get_tuple());
	}
	
	template<class T, class... Types>
		decltype(auto) get(augs::trivially_copyable_tuple<Types...>& t) {
		return std::get<T>(t.get_tuple());
	}

	template<class T, class... Types>
	decltype(auto) get(const augs::trivially_copyable_tuple<Types...>& t) {
		return std::get<T>(t.get_tuple());
	}
}