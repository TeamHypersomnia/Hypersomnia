#pragma once
#include <tuple>
#include "augs/templates/memcpy_safety.h"

namespace augs {
	template <class... Types>
	class trivial_tuple {
		typedef std::tuple<Types...> tuple_type;

		char buf[sizeof(std::tuple<Types...>)];
	public:
		static_assert(are_types_memcpy_safe_v<Types...>, "one of the types is not trivial!");

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
	decltype(auto) get(augs::trivial_tuple<Types...>& t) {
		return std::get<idx>(t.get_tuple());
	}

	template<int idx, class... Types>
	decltype(auto) get(const augs::trivial_tuple<Types...>& t) {
		return std::get<idx>(t.get_tuple());
	}
	
	template<class T, class... Types>
		decltype(auto) get(augs::trivial_tuple<Types...>& t) {
		return std::get<T>(t.get_tuple());
	}

	template<class T, class... Types>
	decltype(auto) get(const augs::trivial_tuple<Types...>& t) {
		return std::get<T>(t.get_tuple());
	}
}