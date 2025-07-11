#pragma once
#include <cstddef>
#include "augs/misc/pool/pooled_object_id.h"
#include "application/setups/editor/resources/editor_resource_id.h"
#include "augs/ensure.h"

template <class E>
struct editor_specific_pool_typed_resource_id {
	editor_resource_pool_id raw;
};

template <class E>
struct editor_typed_resource_id {
	using target_type = E;
	editor_resource_pool_id raw;
	bool is_official = false;

	mutable std::string _serialized_resource_name = "";

	static editor_typed_resource_id<E> from_raw(const editor_resource_pool_id& raw, const bool is_official) {
		return { raw, is_official, {} };
	}

	static auto get_type_id() {
		return editor_resource_type_id::of<E>();
	}

	static editor_typed_resource_id<E> from_generic(const editor_resource_id& id) {
		ensure(get_type_id() == id.type_id);
		return { id.raw, id.is_official };
	}

	explicit operator editor_resource_id() const {
		return { raw, get_type_id(), is_official };
	}

	editor_specific_pool_typed_resource_id<E> to_specific_pool() const {
		return { raw };
	}

	bool operator==(const editor_typed_resource_id<E>& b) const {
		const bool serialization_context = !_serialized_resource_name.empty() || !b._serialized_resource_name.empty();

		if (serialization_context) {
			return _serialized_resource_name == b._serialized_resource_name;
		}

		return raw == b.raw && is_official == b.is_official;
	}

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
