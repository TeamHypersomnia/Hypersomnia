#pragma once
#include <type_traits>
#include "augs/ensure.h"
#include "augs/misc/pool/pooled_object_id.h"
#include "augs/build_settings/platform_defines.h"

#include "game/organization/all_components_declaration.h"
#include "game/transcendental/entity_id_declaration.h"
#include "game/transcendental/cosmic_types.h"

using cosmic_entity = component_list_t<cosmic_aggregate>;

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

struct unversioned_entity_id : public cosmic_object_unversioned_id<cosmic_entity> {
	using base = cosmic_object_unversioned_id<cosmic_entity>;

	unversioned_entity_id(const base b = base()) : base(b) {}
};

struct child_entity_id;

struct entity_id : public cosmic_object_pool_id<cosmic_entity> {
	using base = cosmic_object_pool_id<cosmic_entity>;
	// GEN INTROSPECTOR struct entity_id
	// INTROSPECT BASE cosmic_object_pool_id<cosmic_entity>
	// END GEN INTROSPECTOR

	entity_id(const child_entity_id c);
	entity_id(const base b = base()) : base(b) {}

	operator unversioned_entity_id() const {
		return static_cast<unversioned_entity_id::base>(*static_cast<const base*>(this));
	}
};

struct child_entity_id : entity_id {
	// GEN INTROSPECTOR struct child_entity_id
	// INTROSPECT BASE entity_id
	// END GEN INTROSPECTOR

	using base = entity_id;

	child_entity_id(const base b = base()) : base(b) {}

	using base::operator unversioned_entity_id;
};

FORCE_INLINE entity_id::entity_id(const child_entity_id c) : entity_id(*static_cast<const entity_id*>(&c)) {}

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

	template <>
	struct hash<entity_id> {
		std::size_t operator()(const entity_id v) const {
			return hash<entity_id::base>()(v);
		}
	};

	template <>
	struct hash<unversioned_entity_id> {
		std::size_t operator()(const unversioned_entity_id v) const {
			return hash<unversioned_entity_id::base>()(v);
		}
	};
}
