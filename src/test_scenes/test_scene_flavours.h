#pragma once
#include "augs/templates/enum_introspect.h"
#include "augs/templates/type_map.h"
#include "augs/misc/pool/pool_allocate.h"
#include "test_scenes/test_id_to_pool_id.h"

#include "game/cosmos/entity_flavour_id.h"

#include "game/assets/all_logical_assets.h"
#include "game/common_state/entity_flavours.h"

#include "view/viewables/image_in_atlas.h"

#include "augs/string/format_enum.h"

#include "test_scenes/ingredients/add_sprite.h"
#include "view/viewables/image_cache.h"

#include "test_scenes/test_scene_flavour_ids.h"

using test_flavours_map = type_map<
	type_pair<test_controlled_characters, controlled_character>,
	type_pair<test_plain_sprited_bodies, plain_sprited_body>,
	type_pair<test_shootable_weapons, shootable_weapon>,
	type_pair<test_melee_weapons, melee_weapon>,
	type_pair<test_shootable_charges, shootable_charge>,
	type_pair<test_static_decorations, static_decoration>,
	type_pair<test_dynamic_decorations, dynamic_decoration>,
	type_pair<test_wandering_pixels_decorations, wandering_pixels_decoration>,
	type_pair<test_static_lights, static_light>,
	type_pair<test_hand_explosives, hand_explosive>,
	type_pair<test_plain_missiles, plain_missile>,
	type_pair<test_finishing_traces, finishing_trace>,
	type_pair<test_container_items, container_item>,
	type_pair<test_remnant_bodies, remnant_body>,
	type_pair<test_sound_decorations, sound_decoration>,
	type_pair<test_particles_decorations, particles_decoration>,
	type_pair<test_point_markers, point_marker>,
	type_pair<test_area_markers, area_marker>,
	type_pair<test_explosion_bodies, explosion_body>,
	type_pair<test_touch_collectibles, touch_collectible>,
	type_pair<test_tool_items, tool_item>,
	type_pair<test_decal_decorations, decal_decoration>
>;

#if TODO_CARS
TRUCK_FRONT,
TRUCK_INTERIOR,
TRUCK_LEFT_WHEEL,
TRUCK_ENGINE_BODY,
#endif

template <class T>
inline auto to_raw_flavour_id(const T enum_id) {
	return to_pool_id<raw_entity_flavour_id>(enum_id);
}

template <class T>
inline auto to_entity_flavour_id(const T enum_id) {
	using E = test_flavours_map::at<T>;
	return typed_entity_flavour_id<E>(to_raw_flavour_id(enum_id));
}

template <class T>
auto& get_test_flavour(const all_entity_flavours& flavours, const T enum_id) {
	using E = test_flavours_map::at<T>;

	auto& into = flavours.get_for<E>();
	const auto flavour_id = to_raw_flavour_id(enum_id);
	return into[flavour_id];
}

float get_penetration(const test_shootable_weapons w);

template <class T>
auto& get_test_flavour(all_entity_flavours& flavours, const T enum_id) {
	using E = test_flavours_map::at<T>;

	auto& into = flavours.get_for<E>();

	if (into.empty()) {
		into.reserve(enum_count(T()));

		augs::for_each_enum_except_bounds([&into](const T t) {
			const auto new_allocation = into.allocate();
			(void)t;
			(void)new_allocation;
			const auto rt = to_raw_flavour_id(t);

			ensure_eq(rt.indirection_index, new_allocation.key.indirection_index);
			ensure_eq(rt.version, new_allocation.key.version);

			(void)rt;
		});
	}

	const auto flavour_id = to_raw_flavour_id(enum_id);
	auto& new_flavour = into[flavour_id];
	new_flavour.template get<invariants::text_details>().name = format_enum(enum_id);
	new_flavour.template get<invariants::text_details>().resource_id = to_lowercase(augs::enum_to_string(enum_id));

	if constexpr(std::is_same_v<test_shootable_weapons, T>) {
		new_flavour.template get<invariants::gun>().basic_penetration_distance = get_penetration(enum_id);
	}

	return new_flavour;
}

struct populate_flavours_input {
	const loaded_image_caches_map& caches;
	const plain_animations_pool& plain_animations;
	all_entity_flavours& flavours;

	auto flavour_with_sprite_maker() const {
		return test_flavours::flavour_with_sprite_maker(flavours, caches);
	}
};

namespace test_flavours {
	void populate_other_flavours(populate_flavours_input);
	void populate_car_flavours(populate_flavours_input);
	void populate_crate_flavours(populate_flavours_input);
	void populate_decoration_flavours(populate_flavours_input);
	void populate_melee_flavours(populate_flavours_input);
	void populate_backpack_flavours(populate_flavours_input);
	void populate_gun_flavours(populate_flavours_input);
	void populate_grenade_flavours(populate_flavours_input);
	void populate_character_flavours(populate_flavours_input);
}

void populate_test_scene_flavours(populate_flavours_input);
