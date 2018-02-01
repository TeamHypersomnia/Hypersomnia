#pragma once
#include "game/organization/all_entity_types.h"

template <class entity_flavour>
struct typed_entity_id : public cosmic_object_pool_id<cosmic_entity> {
	using base = cosmic_object_pool_id<cosmic_entity>;

	// GEN INTROSPECTOR struct typed_entity_id class entity_flavour
	// INTROSPECT BASE cosmic_object_pool_id<cosmic_entity>
	// END GEN INTROSPECTOR

	typed_entity_id(const child_entity_id c);
	typed_entity_id(const base b = base()) : base(b) {}

	operator entity_id() const {

	}
}

using entity_flavour_id = type_in_list_id<entity_flavour_list_t<type_list>>;

using basic_flavour_id = unsigned;

struct entity_flavour_id {
	entity_flavour_id flavour_id;
	basic_flavour_id flavour_id;
};

template <class T>
struct typed_entity_flavour_id {
	basic_flavour_id flavour_id;
};	

template <bool is_const, class entity_flavour, class... accessed_components>
class typed_entity_handle {
	static constexpr constrained_access = sizeof...(accessed_components) > 0;

	template <class = std::enable_if_t<!constrained_access>>
	operator basic_entity_handle<is_const>() const {

	}
};

