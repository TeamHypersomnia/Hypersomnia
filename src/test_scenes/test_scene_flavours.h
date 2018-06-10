#pragma once
#include "augs/templates/enum_introspect.h"
#include "augs/templates/type_map.h"
#include "test_scenes/test_id_to_pool_id.h"

#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/cosmic_functions.h"

#include "game/assets/all_logical_assets.h"
#include "game/common_state/entity_flavours.h"

#include "view/viewables/image_in_atlas.h"

#include "augs/string/format_enum.h"

#include "view/viewables/image_cache.h"

enum class test_static_lights {
	// GEN INTROSPECTOR enum class test_static_lights
	STRONG_LAMP,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_sprite_decorations {
	// GEN INTROSPECTOR enum class test_sprite_decorations
	HAVE_A_PLEASANT,
	SOIL,
	ROAD_DIRT,
	ROAD,
	FLOOR,
	AWAKENING,
	METROPOLIS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_controlled_characters {
	// GEN INTROSPECTOR enum class test_controlled_characters
	METROPOLIS_SOLDIER,
	RESISTANCE_SOLDIER,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_plain_invisible_bodys {

};

enum class test_plain_sprited_bodys {
	// GEN INTROSPECTOR enum class test_plain_sprited_bodys
	CRATE,
	CYAN_SHELL,
	STEEL_SHELL,
	BRICK_WALL,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_shootable_weapons {
	// GEN INTROSPECTOR enum class test_shootable_weapons
	SAMPLE_RIFLE,
	VINDICATOR,
	LEWSII,
	KEK9,
	AMPLIFIER_ARM,
	DATUM_GUN,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_shootable_charges {
	// GEN INTROSPECTOR enum class test_shootable_charges
	CYAN_CHARGE,
	STEEL_CHARGE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_wandering_pixels_decorations {
	// GEN INTROSPECTOR enum class test_wandering_pixels_decorations
	WANDERING_PIXELS,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_throwable_explosives {
	// GEN INTROSPECTOR enum class test_throwable_explosives
	FORCE_GRENADE,
	PED_GRENADE,
	INTERFERENCE_GRENADE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_plain_missiles {
	// GEN INTROSPECTOR enum class test_plain_missiles
	CYAN_ROUND,
	STEEL_ROUND,
	ELECTRIC_MISSILE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_finishing_traces {
	// GEN INTROSPECTOR enum class test_finishing_traces
	CYAN_ROUND_FINISHING_TRACE,
	STEEL_ROUND_FINISHING_TRACE,
	ELECTRIC_MISSILE_FINISHING_TRACE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_container_items {
	// GEN INTROSPECTOR enum class test_container_items
	SAMPLE_BACKPACK,
	BROWN_BACKPACK,
	SAMPLE_MAGAZINE,
	LEWSII_MAG,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_explosive_missiles {

};

using test_flavours_map = type_map<
	type_pair<test_controlled_characters, controlled_character>,
	type_pair<test_plain_invisible_bodys, plain_invisible_body>,
	type_pair<test_plain_sprited_bodys, plain_sprited_body>,
	type_pair<test_shootable_weapons, shootable_weapon>,
	type_pair<test_shootable_charges, shootable_charge>,
	type_pair<test_sprite_decorations, sprite_decoration>,
	type_pair<test_wandering_pixels_decorations, wandering_pixels_decoration>,
	type_pair<test_static_lights, static_light>,
	type_pair<test_throwable_explosives, throwable_explosive>,
	type_pair<test_plain_missiles, plain_missile>,
	type_pair<test_finishing_traces, finishing_trace>,
	type_pair<test_container_items, container_item>,
	type_pair<test_explosive_missiles, explosive_missile>
>;

#if TODO
TRUCK_FRONT,
TRUCK_INTERIOR,
TRUCK_LEFT_WHEEL,
TRUCK_ENGINE_BODY,
URBAN_CYAN_MACHETE
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
auto transform_setter(const T& where) {
	return [where](const auto handle) {
		handle.set_logic_transform(where);		
	};
}

template <class C, class E, class F>
auto create_test_scene_entity(C& cosm, const E enum_flavour, F&& callback) {
	return cosmic::specific_create_entity(cosm, to_entity_flavour_id(enum_flavour), std::forward<F>(callback));
}

template <class C, class E>
auto create_test_scene_entity(C& cosm, const E enum_flavour, const vec2 pos) {
	return cosmic::specific_create_entity(cosm, to_entity_flavour_id(enum_flavour), transform_setter(pos));
}

template <class C, class E>
auto create_test_scene_entity(C& cosm, const E enum_flavour, const components::transform where) {
	return cosmic::specific_create_entity(cosm, to_entity_flavour_id(enum_flavour), transform_setter(where));
}

template <class C, class E>
auto create_test_scene_entity(C& cosm, const E enum_flavour) {
	return cosmic::specific_create_entity(cosm, to_entity_flavour_id(enum_flavour), [](const auto) {});
}

template <class T>
auto& get_test_flavour(all_entity_flavours& flavours, const T enum_id) {
	using E = test_flavours_map::at<T>;

	auto& into = flavours.get_for<E>();

	if (into.empty()) {
		into.reserve(enum_count(T()));

		augs::for_each_enum_except_bounds([&into](const T t) {
			const auto new_allocation = into.allocate();
			ensure_eq(to_raw_flavour_id(t), new_allocation.key);
		});
	}

	const auto flavour_id = to_raw_flavour_id(enum_id);
	auto& new_flavour = into[flavour_id];
	new_flavour.template get<invariants::text_details>().name = format_enum(enum_id);

	return new_flavour;
}

struct populate_flavours_input {
	const loaded_image_caches_map& caches;
	const plain_animations_pool& plain_animations;
	all_entity_flavours& flavours;
};

namespace test_flavours {
	void populate_other_flavours(populate_flavours_input);
	void populate_car_flavours(populate_flavours_input);
	void populate_crate_flavours(populate_flavours_input);
	void populate_melee_flavours(populate_flavours_input);
	void populate_backpack_flavours(populate_flavours_input);
	void populate_gun_flavours(populate_flavours_input);
	void populate_grenade_flavours(populate_flavours_input);
	void populate_character_flavours(populate_flavours_input);
}

void populate_test_scene_flavours(populate_flavours_input);
