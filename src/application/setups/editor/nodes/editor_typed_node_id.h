#pragma once
#include "augs/misc/pool/pooled_object_id.h"

using editor_node_pool_size_type = unsigned short;
using editor_node_pool_id = augs::pooled_object_id<editor_node_pool_size_type>;

template <class E>
struct editor_typed_node_id {
	using target_type = E;
	editor_node_pool_id raw;

	bool operator==(const editor_typed_node_id<E>&) const = default;

	void unset() {
		raw.unset();
	}

	bool is_set() const {
		return raw.is_set();
	}
};
