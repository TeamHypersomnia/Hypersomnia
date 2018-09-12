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
#include "game/components/movement_path_component.h"
#include "game/components/missile_component.h"
#include "game/components/attitude_component.h"
#include "game/modes/arena_mode.h"
#include "view/maybe_official_path_declaration.h"
#include "game/detail/inventory/requested_equipment.h"

#include "application/setups/editor/property_editor/property_editor_structs.h"

struct sound_meta;
struct image_meta;

template <class cmd_type>
using field_type_id_t = decltype(decltype(cmd_type::field)::type_id);

template <class cmd_type>
using property_field_type_id_t = field_type_id_t<decltype(cmd_type::property_id)>;

using entity_field_type_id = type_in_list_id<
	type_list<
		augs::trivial_type_marker,
		only_pick_these_items_vector,
		specific_hostile_entities_vector,
		friction_connection_vector,
		damage_owners_vector
	>
>;

using flavour_field_type_id = type_in_list_id<
	type_list<
		augs::trivial_type_marker,
		std::string,
		convex_partitioned_shape::poly_vector_type,
		convex_partitioned_shape::convex_poly,
		wandering_pixels_frames,
		remnant_flavour_vector
	>
>;

using cosmos_common_field_type_id = type_in_list_id<
	type_list<
		augs::trivial_type_marker,
		std::string
	>
>;

using asset_field_type_id = type_in_list_id<
	type_list<
		augs::trivial_type_marker,
		std::string,
		convex_partitioned_shape::poly_vector_type,
		convex_partitioned_shape::convex_poly,
		maybe_official_sound_path,
		maybe_official_image_path,
		std::vector<rgba>,
		plain_animation_frames_type,

		std::vector<particles_emission>,

		sound_meta,
		image_meta
	>
>;

using mode_field_type_id  = type_in_list_id<
	type_list<
		augs::trivial_type_marker,
		std::string,
		std::vector<entity_guid>,
		arena_mode_knockouts_vector,
		arena_mode_awards_vector,
		std::vector<mode_player_id>,
		other_equipment_vector,
		spare_ammo_vector
	>
>;

template <class field_type_id>
struct field_address {
	// GEN INTROSPECTOR struct field_address class field_type_id
	unsigned offset = static_cast<unsigned>(-1);
	unsigned element_index = static_cast<unsigned>(-1);
	field_type_id type_id;
	// END GEN INTROSPECTOR

	bool operator==(const field_address& b) const {
		return offset == b.offset && element_index && b.element_index && type_id == b.type_id;
	}

	bool operator!=(const field_address& b) const {
		return !operator==(b);
	}
};

using cosmos_common_field_address = field_address<cosmos_common_field_type_id>;
using flavour_field_address = field_address<flavour_field_type_id>;
using entity_field_address = field_address<entity_field_type_id>;
using asset_field_address = field_address<asset_field_type_id>;
using mode_field_address = field_address<mode_field_type_id>;

template <class I, class M>
auto get_type_id_for_field() {
	I id;

	if constexpr(can_access_data_v<M>) {
		id.template set<M>();
	}
	else if constexpr(is_one_of_list_v<M, typename I::list_type>) {
		id.template set<M>();
	}
	else {
		static_assert(std::is_trivially_copyable_v<M>);
		id.template set<augs::trivial_type_marker>();
	}

	return id;
}

template <class I, class O, class M>
auto make_field_address(const O& object, const M& member) {
	field_address<I> result;

	result.type_id = get_type_id_for_field<I, M>();
	result.offset = static_cast<unsigned>(
		reinterpret_cast<const std::byte*>(std::addressof(member))
		- reinterpret_cast<const std::byte*>(std::addressof(object))
	);

	return result;
}


template <class I, class M>
auto make_field_address(const std::size_t offset) {
	field_address<I> result;

	result.type_id = get_type_id_for_field<I, M>();
	result.offset = static_cast<unsigned>(offset);

	return result;
}

#define MACRO_MAKE_FLAVOUR_FIELD_ADDRESS(a,b) make_field_address<flavour_field_type_id, decltype(a::b)>(augs_offsetof(a,b))
#define MACRO_MAKE_ASSET_FIELD_ADDRESS(a,b) make_field_address<asset_field_type_id, decltype(a::b)>(augs_offsetof(a,b))
