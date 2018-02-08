#pragma once
#include <string>
#include "game/organization/all_entity_types_declaration.h"

class entity_flavour;
struct entity_flavours;

using raw_entity_flavour_id = zeroed_pod<unsigned>;

template <class...>
struct constrained_entity_flavour_id; 

using entity_flavour_id = constrained_entity_flavour_id<>;

template <class... C>
struct constrained_entity_flavour_id {
	using matching_types = all_entity_types_having<C...>;

	raw_entity_flavour_id raw;
	entity_type_id type_id;

	operator entity_flavour_id() const {
		return { raw, type_id };
	}
};

template <class E>
struct typed_entity_flavour_id {
	raw_entity_flavour_id raw;

	typed_entity_flavour_id() = default;
	explicit typed_entity_flavour_id(raw_entity_flavour_id id) : id(id) {};

	operator raw_entity_flavour_id() const {
		return raw;
	}

	template <
		class... C,
		class V = std::enable_if_t<has_invariants_or_components_v<E, Args...>>
	>
	operator constrained_entity_flavour_id<C...>() const {
		entity_type_id type_id;
		type_id.set<E>();

		return { raw, type_id };
	}
};

using entity_name_type = std::wstring;
