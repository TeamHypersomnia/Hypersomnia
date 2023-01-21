#pragma once
#include "augs/misc/pool/pooled_object_id.h"
#include "application/setups/editor/nodes/editor_node_id.h"

template <class E>
struct editor_typed_node_id {
	using target_type = E;
	editor_node_pool_id raw;

	void set(const editor_node_pool_id& new_raw) {
		raw = new_raw;
	}

	static editor_typed_node_id<E> from_generic(const editor_node_id& id) {
		return { id.raw };
	}

	auto get_type_id() const {
		return editor_node_type_id::of<E>();
	}

	explicit operator editor_node_id() const {
		return { raw, get_type_id() };
	}

	bool operator==(const editor_typed_node_id<E>&) const = default;

	void unset() {
		raw.unset();
	}

	bool is_set() const {
		return raw.is_set();
	}
};
