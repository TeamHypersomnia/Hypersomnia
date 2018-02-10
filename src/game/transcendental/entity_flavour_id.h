#pragma once
#include <string>
#include "augs/zeroed_pod.h"
#include "game/transcendental/entity_type_traits.h"
#include "game/organization/all_entity_types_declaration.h"

using raw_entity_flavour_id = zeroed_pod<unsigned>;

template <class...>
struct constrained_entity_flavour_id; 

using entity_flavour_id = constrained_entity_flavour_id<>;

template <class... C>
struct constrained_entity_flavour_id {
	using matching_types = all_entity_types_having<C...>;

	// GEN INTROSPECTOR struct constrained_entity_flavour_id class... C
	raw_entity_flavour_id raw;
	entity_type_id type_id;
	// END GEN INTROSPECTOR

	operator entity_flavour_id() const {
		return { raw, type_id };
	}
};

template <class E>
struct typed_entity_flavour_id {
	raw_entity_flavour_id raw;

	typed_entity_flavour_id() = default;
	explicit typed_entity_flavour_id(raw_entity_flavour_id raw) : raw(raw) {};

	operator raw_entity_flavour_id() const {
		return raw;
	}

	template <
		class... C,
		class V = std::enable_if_t<has_invariants_or_components_v<E, C...>>
	>
	operator constrained_entity_flavour_id<C...>() const {
		entity_type_id type_id;
		type_id.set<E>();

		return { raw, type_id };
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
