#pragma once
#include "augs/misc/pool/pooled_object_id.h"
#include "application/setups/editor/nodes/editor_node_id.h"

template <class E>
struct editor_typed_node_id {
	using target_type = E;
	editor_node_pool_id raw;

	explicit operator editor_node_id() const {
		return { raw, editor_node_type_id::of<E>() };
	}

	bool operator==(const editor_typed_node_id<E>&) const = default;

	void unset() {
		raw.unset();
	}

	bool is_set() const {
		return raw.is_set();
	}
};
