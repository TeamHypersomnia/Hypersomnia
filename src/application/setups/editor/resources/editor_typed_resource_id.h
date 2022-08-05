#pragma once
#include "augs/misc/pool/pooled_object_id.h"
#include "application/setups/editor/resources/editor_resource_id.h"

template <class E>
struct editor_specific_pool_typed_resource_id {
	editor_resource_pool_id raw;
};

template <class E>
struct editor_typed_resource_id {
	using target_type = E;
	editor_resource_pool_id raw;
	bool is_official = false;

	static editor_typed_resource_id<E> from_raw(const editor_resource_pool_id& raw, const bool is_official) {
		return { raw, is_official };
	}

	static editor_typed_resource_id<E> from_generic(const editor_resource_id& id) {
		return { id.raw, id.is_official };
	}

	explicit operator editor_resource_id() const {
		return { raw, editor_resource_type_id::of<E>(), is_official };
	}

	editor_specific_pool_typed_resource_id<E> to_specific_pool() const {
		return { raw };
	}

	bool operator==(const editor_typed_resource_id<E>&) const = default;

	void unset() {
		raw.unset();
		is_official = false;
	}

	bool is_set() const {
		return raw.is_set();
	}

	auto hash() const {
		return augs::hash_multiple(raw, is_official);
	}
};

namespace std {
	template <class E>
	struct hash<editor_typed_resource_id<E>> {
		std::size_t operator()(const editor_typed_resource_id<E> v) const {
			return v.hash();
		}
	};
}
