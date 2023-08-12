#pragma once
#include <type_traits>
#include <compare>
#include "augs/misc/pool/pooled_object_id.h"
#include "augs/build_settings/compiler_defines.h"

#include "game/cosmos/entity_id_declaration.h"

#include "game/organization/all_entity_types.h"

struct unversioned_entity_id {
	// GEN INTROSPECTOR struct unversioned_entity_id
	unversioned_entity_id_base raw;
	entity_type_id type_id;
	// END GEN INTROSPECTOR

	unversioned_entity_id() = default;

	unversioned_entity_id(
		const unversioned_entity_id_base id,
		const entity_type_id type_id
	) : 
		raw(id),
		type_id(type_id)
	{}

	bool is_set() const {
		return raw.is_set() && type_id.is_set();
	}

	void unset() {
		raw.unset();
		type_id.unset();
	}

	bool operator==(const unversioned_entity_id& b) const = default;

	auto hash() const {
		return augs::hash_multiple(raw, type_id);
	}
};

struct child_entity_id;

struct entity_id {
	// GEN INTROSPECTOR struct entity_id
	entity_id_base raw;
	entity_type_id type_id;
	// END GEN INTROSPECTOR

	entity_id() = default;

	entity_id(
		const entity_id_base id,
		entity_type_id type_id
	) : 
		raw(id),
		type_id(type_id)
	{}

	entity_id(
		const child_entity_id& id
	);

	auto get_type_id() const {
		return type_id;
	}

	static auto dead() {
		return entity_id();
	}

	bool operator==(const entity_id& b) const noexcept = default;

	void unset() {
		raw.unset();
		type_id.unset();
	}

	bool is_set() const {
		return raw.is_set() && type_id.is_set();
	}

	unversioned_entity_id to_unversioned() const {
		return { raw, type_id };
	}

	operator unversioned_entity_id() const {
		return { raw, type_id };
	}

	friend std::ostream& operator<<(std::ostream& out, const entity_id x);

	bool operator<(const entity_id& b) const {
		if (type_id == b.type_id) {
			return raw < b.raw;
		}

		return type_id < b.type_id;
	}

	auto hash() const {
		return augs::hash_multiple(raw, type_id);
	}
}; 

template <class E>
struct typed_entity_id {
	using raw_type = entity_id_base;
	using used_entity_type = E;

	// GEN INTROSPECTOR struct typed_entity_id class E
	raw_type raw;
	// END GEN INTROSPECTOR

	typed_entity_id() = default;
	typed_entity_id(const entity_id b) = delete;
	explicit typed_entity_id(const raw_type b) : raw(b) {}

	operator entity_id() const {
		return { raw, entity_type_id::of<E>() };
	}

	unversioned_entity_id to_unversioned() const {
		return { raw, entity_type_id::of<E>() };
	}

	bool operator==(const entity_id b) const {
		return entity_id(*this) == b;
	}

	bool operator!=(const entity_id b) const {
		return !operator==(b);
	}

	bool operator==(const typed_entity_id<E> b) const {
		return raw == b.raw;
	}

	bool operator!=(const typed_entity_id<E> b) const {
		return !operator==(b);
	}

	bool operator<(const typed_entity_id<E>& b) const {
		return raw < b.raw;
	}

	bool is_set() const {
		return raw.is_set();
	}

	auto hash() const {
		return augs::hash_multiple(raw);
	}

	auto get_type_id() const {
		return entity_type_id::of<E>();
	}
}; 

template <class T>
const std::string& get_type_name();

template <class E>
std::ostream& operator<<(std::ostream& out, const typed_entity_id<E>& x) {
	if (!x.is_set()) {
		return out << "(unset)";
	}

	return out << get_type_name<E>() << "-" << x.raw;
}

struct child_entity_id : entity_id {
	using base = entity_id;
	using introspect_base = base;

	child_entity_id(const entity_id id = entity_id()) : entity_id(id) {}
	using base::operator unversioned_entity_id;
};

inline entity_id::entity_id(
	const child_entity_id& id
) : entity_id(static_cast<const entity_id&>(id)) {}


namespace std {
	template <>
	struct hash<entity_id> {
		std::size_t operator()(const entity_id v) const {
			return v.hash();
		}
	};

	template <class E>
	struct hash<typed_entity_id<E>> {
		std::size_t operator()(const typed_entity_id<E> v) const {
			return v.hash();
		}
	};

	template <>
	struct hash<unversioned_entity_id> {
		std::size_t operator()(const unversioned_entity_id v) const {
			return v.hash();
		}
	};
}
