#pragma once
#include <type_traits>
#include "augs/ensure.h"
#include "augs/misc/pool/pooled_object_id.h"
#include "augs/build_settings/platform_defines.h"

#include "game/transcendental/entity_id_declaration.h"
#include "game/transcendental/cosmic_types.h"

struct entity_guid {
	using guid_value_type = unsigned;
	// GEN INTROSPECTOR struct entity_guid
	guid_value_type value = 0u;
	// END GEN INTROSPECTOR

	entity_guid(const guid_value_type b = 0u) : value(b) {}
	
	entity_guid& operator=(const guid_value_type b) {
		value = b;
		return *this;
	}

	operator guid_value_type() const {
		return value;
	}
};

struct child_entity_id : entity_id {
	// GEN INTROSPECTOR struct child_entity_id
	// INTROSPECT BASE entity_id
	// END GEN INTROSPECTOR

	using base = entity_id;
	using entity_id::entity_id;
	child_entity_id(entity_id id = entity_id()) : entity_id(id) {}
	using base::operator unversioned_entity_id;
};

inline auto linear_cache_key(const entity_id id) {
	ensure(id.is_set());
	return id.indirection_index;
}

inline auto linear_cache_key(const unversioned_entity_id id) {
	ensure(id.is_set());
	return id.indirection_index;
}

namespace std {
	template <>
	struct hash<entity_guid> {
		std::size_t operator()(const entity_guid v) const {
			return hash<entity_guid::guid_value_type>()(v.value);
		}
	};
}
