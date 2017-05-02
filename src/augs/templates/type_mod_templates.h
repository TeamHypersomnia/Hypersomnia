#pragma once
#include <vector>

namespace augs {
	template <class, size_t>
	class constant_size_vector;
}

template<class T>
struct make_vector {
	typedef std::vector<T> type;
};

template <size_t I>
struct of_size {
	template <class T>
	struct make_constant_vector {
		typedef augs::constant_size_vector<T, I> type;
	};
};