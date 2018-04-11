#pragma once
#include "game/assets/ids/asset_ids.h"

template <class P, class T>
inline P to_pool_id(const T id) {
	/* 
		This is a massive sleight of hand. 
		We predict here what identificators will the pool return.
	*/

	P result;
	result.indirection_index = static_cast<unsigned>(id);
	return result;
}
