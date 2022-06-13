#pragma once
#include "augs/misc/pool/pooled_object_id.h"

using editor_resource_pool_size_type = unsigned short;
using editor_resource_pool_id = augs::pooled_object_id<editor_resource_pool_size_type>;

template <class E>
struct editor_typed_resource_id {
	editor_resource_pool_id raw;

	bool operator==(const editor_typed_resource_id<E>&) const = default;

	void unset() {
		raw.unset();
	}

	bool is_set() const {
		return raw.is_set();
	}
};
