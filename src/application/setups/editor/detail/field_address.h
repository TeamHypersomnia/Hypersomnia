#pragma once
#include <cstddef>
#include "augs/filesystem/path_declaration.h"

#include "game/components/shape_polygon_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/sentience_component.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"

using edited_field_type_id = type_in_list_id<
	type_list<
		augs::trivial_type_marker,
		std::string,
		convex_partitioned_shape::poly_vector_type,
		convex_partitioned_shape::convex_poly,
		maybe_official_path,
		std::vector<rgba>,
		friction_connection_vector
	>
>;

struct field_address {
	// GEN INTROSPECTOR struct field_address
	unsigned offset = static_cast<unsigned>(-1);
	unsigned element_index = static_cast<unsigned>(-1);
	edited_field_type_id type_id;
	// END GEN INTROSPECTOR
};

template <class M>
auto get_type_id_for_field() {
	edited_field_type_id id;

	if constexpr(std::is_trivially_copyable_v<M>) {
		id.set<augs::trivial_type_marker>();
	}
	else {
		id.set<M>();
	}

	return id;
}

template <class O, class M>
auto make_field_address(const O& object, const M& member) {
	field_address result;

	result.type_id = get_type_id_for_field<M>();
	result.offset = static_cast<unsigned>(
		reinterpret_cast<const std::byte*>(std::addressof(member))
		- reinterpret_cast<const std::byte*>(std::addressof(object))
	);

	return result;
}

template <class O, class M>
auto make_field_address(const M O::* const member) {
	static const O o;
	return make_field_address(o, o.*member);
}
