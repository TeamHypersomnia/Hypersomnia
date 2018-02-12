#pragma once
#include <string>
#include "augs/zeroed_pod.h"
#include "augs/templates/hash_templates.h"
#include "game/transcendental/entity_type_traits.h"
#include "game/organization/all_entity_types_declaration.h"

using raw_entity_flavour_id = zeroed_pod<unsigned>;

template <class...>
struct constrained_entity_flavour_id; 

using entity_flavour_id = constrained_entity_flavour_id<>;

template <class... C>
struct constrained_entity_flavour_id {
	using matching_types = entity_types_having_all_of<C...>;

	// GEN INTROSPECTOR struct constrained_entity_flavour_id class... C
	raw_entity_flavour_id raw;
	entity_type_id type_id;
	// END GEN INTROSPECTOR

	operator bool() const {
		return raw != raw_entity_flavour_id();
	}

	operator entity_flavour_id() const {
		return { raw, type_id };
	}
};

template <class E>
struct typed_entity_flavour_id {
	raw_entity_flavour_id raw;

	typed_entity_flavour_id() = default;
	explicit typed_entity_flavour_id(raw_entity_flavour_id raw) : raw(raw) {};

	operator bool() const {
		return raw != raw_entity_flavour_id();
	}

	template <
		class... C,
		class V = std::enable_if_t<has_all_of_v<E, C...>>
	>
	operator constrained_entity_flavour_id<C...>() const {
		return { raw, entity_type_id::of<E> };
	}
};

using entity_name_type = std::wstring;

namespace std {
	template <>
	struct hash<entity_flavour_id> {
		std::size_t operator()(const entity_flavour_id v) const {
			return augs::simple_two_hash(v.raw, v.type_id);
		}
	};

	template <class T>
	struct hash<typed_entity_flavour_id<T>> {
		std::size_t operator()(const typed_entity_flavour_id<T> v) const {
			return augs::simple_two_hash(typeid(T).hash_code(), v.raw);
		}
	};
}
