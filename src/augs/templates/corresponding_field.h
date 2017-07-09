#pragma once
#include <type_traits>

namespace augs {
	template <class T, class A, class B>
	auto& get_corresponding_field(T& field_of_a, A& a, B& b) {
		constexpr auto C = is_const_ref_v<decltype(b)>;

		return 					
			*reinterpret_cast<maybe_const_ptr_t<C, std::remove_const_t<T>>>(
				reinterpret_cast<maybe_const_ptr_t<C, char>>(&b)
				+ (
					reinterpret_cast<const char*>(&field_of_a)
					- reinterpret_cast<const char*>(&a)
				)
			)
		;
	}
}