#pragma once
#include "augs/misc/trivially_copyable_pair.h"

namespace augs {
	template<class T>
	using minmax = trivially_copyable_pair<T, T>;

	template<class T>
	using random_bound = trivially_copyable_pair<minmax<T>, minmax<T>>;
}