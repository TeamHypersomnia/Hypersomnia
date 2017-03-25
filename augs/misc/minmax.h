#pragma once
#include "trivially_copyable_pair.h"

namespace augs {
	template<class T>
	using minmax = trivially_copyable_pair<T, T>;
}