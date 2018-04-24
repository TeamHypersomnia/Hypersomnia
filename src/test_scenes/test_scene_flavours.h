#pragma once
#include "augs/templates/type_map.h"

#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/cosmic_functions.h"

#include "game/common_state/entity_flavours.h"

#include "view/viewables/image_in_atlas.h"

#include "augs/string/format_enum.h"

#include "view/viewables/image_cache.h"

enum class test_static_lights {
	// GEN INTROSPECTOR enum class test_static_lights
	STRONG_LAMP
	// END GEN INTROSPECTOR
};

enum class test_sprite_decorations {
	// GEN INTROSPECTOR enum class test_sprite_decorations
	HAVE_A_PLEASANT,
	STREET,
	ROAD_DIRT,
	ROAD,
	FLOOR,
	AWAKENING,
	METROPOLIS
	// END GEN INTROSPECTOR
};

enum class test_controlled_characters {
	// GEN INTROSPECTOR enum class test_controlled_characters
	PLAYER
	// END GEN INTROSPECTOR
};

enum class test_plain_invisible_bodys {
	// GEN INTROSPECTOR enum class test_plain_invisible_bodys
	INVALID
	// END GEN INTROSPECTOR
};

enum class test_plain_sprited_bodys {
	// GEN INTROSPECTOR enum class test_plain_sprited_bodys
	CRATE,
	CYAN_SHELL_DEFINITION,
	BRICK_WALL
	// END GEN INTROSPECTOR
};

enum class test_shootable_weapons {
	// GEN INTROSPECTOR enum class test_shootable_weapons
	SAMPLE_RIFLE,
	KEK9,
	AMPLIFIER_ARM
	// END GEN INTROSPECTOR
};

enum class test_shootable_charges {
	// GEN INTROSPECTOR enum class test_shootable_charges
	CYAN_CHARGE
	// END GEN INTROSPECTOR
};

enum class test_wandering_pixels_decorations {
	// GEN INTROSPECTOR enum class test_wandering_pixels_decorations
	WANDERING_PIXELS
	// END GEN INTROSPECTOR
};

enum class test_throwable_explosives {
	// GEN INTROSPECTOR enum class test_throwable_explosives
	FORCE_GRENADE,
	PED_GRENADE,
	INTERFERENCE_GRENADE
	// END GEN INTROSPECTOR
};

enum class test_plain_missiles {
	// GEN INTROSPECTOR enum class test_plain_missiles
	CYAN_ROUND_DEFINITION,
	AMPLIFIER_ARM_MISSILE,
	ELECTRIC_MISSILE
	// END GEN INTROSPECTOR
};

enum class test_finishing_traces {
	// GEN INTROSPECTOR enum class test_finishing_traces
	CYAN_ROUND_FINISHING_TRACE,
	ENERGY_BALL_FINISHING_TRACE
	// END GEN INTROSPECTOR
};

enum class test_container_items {
	// GEN INTROSPECTOR enum class test_container_items
	SAMPLE_BACKPACK,
	SAMPLE_MAGAZINE
	// END GEN INTROSPECTOR
};

enum class test_explosive_missiles {
	// GEN INTROSPECTOR enum class test_explosive_missiles
	INVALID
	// END GEN INTROSPECTOR
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
	return static_cast<raw_entity_flavour_id>(static_cast<unsigned>(enum_id));
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
auto& get_test_flavour(all_entity_flavours& flavours, const T id) {
	using E = test_flavours_map::at<T>;
	const auto idx = static_cast<std::size_t>(id);

	auto& into = std::get<make_entity_flavours<E>>(flavours);

	if (into.count() < idx + 1) {
		into.resize(idx + 1);
	}

	auto& new_flavour = into.get_flavour(idx);
	new_flavour.template get<invariants::name>().name = format_enum(id);

	return new_flavour;
}

namespace test_flavours {
	void populate_other_flavours(const loaded_image_caches_map& caches, all_entity_flavours& flavours);
	void populate_car_flavours(const loaded_image_caches_map& caches, all_entity_flavours& flavours);
	void populate_crate_flavours(const loaded_image_caches_map& caches, all_entity_flavours& flavours);
	void populate_melee_flavours(const loaded_image_caches_map& caches, all_entity_flavours& flavours);
	void populate_backpack_flavours(const loaded_image_caches_map& caches, all_entity_flavours& flavours);
	void populate_gun_flavours(const loaded_image_caches_map& caches, all_entity_flavours& flavours);
	void populate_grenade_flavours(const loaded_image_caches_map& caches, all_entity_flavours& flavours);
	void populate_character_flavours(const loaded_image_caches_map& caches, all_entity_flavours& flavours);
}

void populate_test_scene_flavours(const loaded_image_caches_map& caches, all_entity_flavours& into);
