#pragma once
#include "augs/misc/simple_pair.h"

namespace augs {
	template<class T>
	using minmax = simple_pair<T, T>;

	template<class T>
	using random_bound = simple_pair<minmax<T>, minmax<T>>;
}