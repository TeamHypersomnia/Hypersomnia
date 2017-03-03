#pragma once
#include <type_traits>

namespace augs {
	template <class T, class F>
	decltype(auto) introspect(
		T& t,
		F f
	) {
		return introspect<std::is_const<T>::value>(t, f);
	}
}
