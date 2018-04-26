#pragma once
#include "game/assets/ids/asset_ids.h"

template <class P, class T>
inline P to_pool_id(const T id) {
	/* 
		This is a massive sleight of hand. 
		We predict here what identificators will be returned by the pool.
	*/

	P result;
	result.indirection_index = static_cast<unsigned>(id);
	result.version = 1;
	return result;
}

template <class T>
auto enum_count(const T t) {
	static_assert(std::is_enum_v<T>);
	return static_cast<std::size_t>(T::COUNT);
}
