#pragma once
#include <cstddef>
#include "augs/build_settings/offsetof.h"
#include "augs/filesystem/path_declaration.h"
#include "game/assets/animation.h"
#include "view/viewables/particle_effect.h"

#include "game/components/shape_polygon_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/sentience_component.h"
#include "game/components/wandering_pixels_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/attitude_component.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"

struct sound_meta;
struct image_meta;

using edited_field_type_id = type_in_list_id<
	type_list<
		augs::trivial_type_marker,
		std::string,
		convex_partitioned_shape::poly_vector_type,
		convex_partitioned_shape::convex_poly,
		maybe_official_sound_path,
		maybe_official_image_path,
		std::vector<rgba>,
		wandering_pixels_frames,
		only_pick_these_items_vector,
		specific_hostile_entities_vector,
		friction_connection_vector,

		plain_animation_frames_type,

		std::vector<particles_emission>,

		sound_meta,
		image_meta
	>
>;

struct field_address {
	// GEN INTROSPECTOR struct field_address
	unsigned offset = static_cast<unsigned>(-1);
	unsigned element_index = static_cast<unsigned>(-1);
	edited_field_type_id type_id;
	// END GEN INTROSPECTOR

	bool operator==(const field_address& b) const {
		return offset == b.offset && element_index && b.element_index && type_id == b.type_id;
	}

	bool operator!=(const field_address& b) const {
		return !operator==(b);
	}
};

template <class M>
auto get_type_id_for_field() {
	edited_field_type_id id;

	if constexpr(can_access_data_v<M>) {
		id.set<M>();
	}
	else if constexpr(is_one_of_list_v<M, edited_field_type_id::list_type>) {
		id.set<M>();
	}
	else {
		static_assert(std::is_trivially_copyable_v<M>);
		id.set<augs::trivial_type_marker>();
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


template <class M>
auto make_field_address(const std::size_t offset) {
	field_address result;

	result.type_id = get_type_id_for_field<M>();
	result.offset = static_cast<unsigned>(offset);

	return result;
}

#define MACRO_MAKE_FIELD_ADDRESS(a,b) make_field_address<decltype(a::b)>(augs_offsetof(a,b))
