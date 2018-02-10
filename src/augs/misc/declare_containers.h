#pragma once

namespace augs {
	template <class Enum, class T, class = void>
	class enum_associative_array;

	template <class T, unsigned, class = void>
	class constant_size_vector;
}

template <unsigned I>
struct of_size {
	template <class T>
	using make_constant_vector = augs::constant_size_vector<T, I>;
};