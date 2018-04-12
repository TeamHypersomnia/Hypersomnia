#pragma once
#include <type_traits>
#include "augs/misc/pool/pooled_object_id.h"
#include "augs/build_settings/platform_defines.h"

#include "game/transcendental/entity_id_declaration.h"

#include "game/organization/all_entity_types.h"

struct entity_guid {
	using guid_value_type = unsigned;
	// GEN INTROSPECTOR struct entity_guid
	guid_value_type value = 0u;
	// END GEN INTROSPECTOR

	entity_guid() = default;
	entity_guid(const guid_value_type b) : value(b) {}
	
	entity_guid& operator=(const entity_guid& b) = default;

	bool operator==(const entity_guid& b) const {
		return value == b.value;
	}

	bool operator!=(const entity_guid& b) const {
		return value != b.value;
	}

	operator guid_value_type() const {
		return value;
	}

	bool is_set() const {
		return *this != entity_guid();
	}

	void unset() {
		*this = {};
	}
};

struct unversioned_entity_id : unversioned_entity_id_base {
	using base = unversioned_entity_id_base;
	// GEN INTROSPECTOR struct unversioned_entity_id
	// INTROSPECT BASE unversioned_entity_id_base
	entity_type_id type_id;
	// END GEN INTROSPECTOR

	auto basic() const {
		return *static_cast<const base*>(this);
	}

	bool is_set() const {
		return base::is_set() && type_id.is_set();
	}

	void unset() {
		*this = unversioned_entity_id();
	}

	bool operator==(const unversioned_entity_id b) const {
		return type_id == b.type_id && basic() == b.basic();
	}

	bool operator!=(const unversioned_entity_id b) const {
		return !operator==(b);
	}

	unversioned_entity_id() = default;

	unversioned_entity_id(
		const base id,
		entity_type_id type_id
	) : 
		base(id),
		type_id(type_id)
	{}

private:
	using base::operator==;
	using base::operator!=;
};

struct child_entity_id;

struct entity_id : entity_id_base {
	using base = entity_id_base;
	// GEN INTROSPECTOR struct entity_id
	// INTROSPECT BASE entity_id_base
	entity_type_id type_id;
	// END GEN INTROSPECTOR
	entity_id() = default;

	entity_id(
		const base id,
		entity_type_id type_id
	) : 
		base(id),
		type_id(type_id)
	{}

	entity_id(
		const child_entity_id& id
	);

	auto basic() const {
		return *static_cast<const base*>(this);
	}

	bool operator==(const entity_id b) const {
		return type_id == b.type_id && basic() == b.basic();
	}

	bool operator!=(const entity_id b) const {
		return !operator==(b);
	}

	void unset() {
		*this = entity_id();
	}

	bool is_set() const {
		return base::is_set() && type_id.is_set();
	}

	explicit operator bool() const {
		return is_set();
	}

	operator unversioned_entity_id() const {
		return { basic(), type_id };
	}

	friend std::ostream& operator<<(std::ostream& out, const entity_id x);

private:
	using base::operator==;
	using base::operator!=;
}; 

template <class E>
struct typed_entity_id : entity_id_base {
	using base = entity_id_base;

	typed_entity_id(const entity_id_base b = {}) : entity_id_base(b) {}

	auto basic() const {
		return *static_cast<const base*>(this);
	}

	operator entity_id() const {
		return { *this, entity_type_id::of<E>() };
	}

	bool operator==(const typed_entity_id<E> b) const {
		return basic() == b.basic();
	}

	bool operator!=(const typed_entity_id<E> b) const {
		return !operator==(b);
	}

private:
	using base::operator==;
	using base::operator!=;
}; 

struct child_entity_id : entity_id {
	// GEN INTROSPECTOR struct child_entity_id
	// INTROSPECT BASE entity_id
	// END GEN INTROSPECTOR

	using base = entity_id;
	child_entity_id(const entity_id id = entity_id()) : entity_id(id) {}
	using base::operator unversioned_entity_id;

	auto basic() const {
		return *static_cast<const base*>(this);
	}
};

inline entity_id::entity_id(
	const child_entity_id& id
) : entity_id(id.basic()) {}

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
			return augs::simple_two_hash(v.basic(), v.type_id);
		}
	};

	template <>
	struct hash<unversioned_entity_id> {
		std::size_t operator()(const unversioned_entity_id v) const {
			return augs::simple_two_hash(v.basic(), v.type_id);
		}
	};
}
