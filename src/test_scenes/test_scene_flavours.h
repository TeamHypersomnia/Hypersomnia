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

enum class test_static_lights {
	// GEN INTROSPECTOR enum class test_static_lights
	STRONG_LAMP,
	AQUARIUM_LAMP,

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
	WATER_ROOM_FLOOR,
	AWAKENING,
	METROPOLIS,

	AQUARIUM_BOTTOM_LAMP_LIGHT,
	AQUARIUM_BOTTOM_LAMP_BODY,

	AQUARIUM_HALOGEN_1_LIGHT,
	AQUARIUM_HALOGEN_1_BODY,

	AQUARIUM_SAND_1,
	AQUARIUM_SAND_2,

	DUNE_BIG,
	DUNE_SMALL,

	AQUARIUM_SAND_EDGE,
	AQUARIUM_SAND_CORNER,

	WATER_COLOR_OVERLAY,
	LAB_WALL_A2,

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

enum class test_plain_sprited_bodys {
	// GEN INTROSPECTOR enum class test_plain_sprited_bodys
	CRATE,
	CYAN_SHELL,
	STEEL_SHELL,
	AO44_SHELL,
	BRICK_WALL,
	AQUARIUM_GLASS,

	AQUARIUM_GLASS_START,

	LAB_WALL,
	LAB_WALL_SMOOTH_END,
	LAB_WALL_CORNER_CUT,
	LAB_WALL_CORNER_SQUARE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_shootable_weapons {
	// GEN INTROSPECTOR enum class test_shootable_weapons
	BILMER2000,
	VINDICATOR,
	LEWSII,
	KEK9,
	SN69,
	AO44,
	PRO90,
	AMPLIFIER_ARM,
	DATUM_GUN,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_shootable_charges {
	// GEN INTROSPECTOR enum class test_shootable_charges
	CYAN_CHARGE,
	STEEL_CHARGE,
	AO44_CHARGE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_wandering_pixels_decorations {
	// GEN INTROSPECTOR enum class test_wandering_pixels_decorations
	WANDERING_PIXELS,

	AQUARIUM_PIXELS_LIGHT,
	AQUARIUM_PIXELS_DIM,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_hand_explosives {
	// GEN INTROSPECTOR enum class test_hand_explosives
	FORCE_GRENADE,
	PED_GRENADE,
	INTERFERENCE_GRENADE,

	BOMB,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_plain_missiles {
	// GEN INTROSPECTOR enum class test_plain_missiles
	CYAN_ROUND,
	STEEL_ROUND,
	AO44_ROUND,
	ELECTRIC_MISSILE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_finishing_traces {
	// GEN INTROSPECTOR enum class test_finishing_traces
	CYAN_ROUND_FINISHING_TRACE,
	STEEL_ROUND_FINISHING_TRACE,
	AO44_ROUND_FINISHING_TRACE,
	ELECTRIC_MISSILE_FINISHING_TRACE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_container_items {
	// GEN INTROSPECTOR enum class test_container_items
	STANDARD_PERSONAL_DEPOSIT,

	SAMPLE_BACKPACK,
	BROWN_BACKPACK,
	STANDARD_MAGAZINE,
	KEK9_MAGAZINE,
	SN69_MAGAZINE,
	AO44_MAGAZINE,
	PRO90_MAGAZINE,
	LEWSII_MAGAZINE,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_explosive_missiles {

};

enum class test_complex_decorations {
	// GEN INTROSPECTOR enum class test_complex_decorations
	ROTATING_FAN,
	YELLOW_FISH,
	DARKBLUE_FISH,
	CYANVIOLET_FISH,
	JELLYFISH,
	DRAGON_FISH,
	RAINBOW_DRAGON_FISH,
	WATER_SURFACE,

	FLOWER_PINK,
	FLOWER_CYAN,

	CONSOLE_LIGHT,

	PINK_CORAL,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_remnant_bodies {
	// GEN INTROSPECTOR enum class test_remnant_bodies
	STEEL_ROUND_REMNANT_1,
	STEEL_ROUND_REMNANT_2,
	STEEL_ROUND_REMNANT_3,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_sound_decorations {
	// GEN INTROSPECTOR enum class test_sound_decorations
	AQUARIUM_AMBIENCE_LEFT,
	AQUARIUM_AMBIENCE_RIGHT,

	HUMMING_DISABLED,
	LOUDY_FAN,

	COUNT
	// END GEN INTROSPECTOR
};

enum class test_particles_decorations {
	// GEN INTROSPECTOR enum class test_particles_decorations
	AQUARIUM_BUBBLES,
	FLOWER_BUBBLES,
	COUNT
	// END GEN INTROSPECTOR
};

enum class test_point_markers {
	// GEN INTROSPECTOR enum class test_point_markers
	FFA_SPAWN,
	METROPOLIS_SPAWN,
	RESISTANCE_SPAWN,
	COUNT
	// END GEN INTROSPECTOR
};

enum class test_box_markers {
	// GEN INTROSPECTOR enum class test_box_markers
	BOMBSITE_A,
	BOMBSITE_B,
	RESISTANCE_BUY_AREA,
	METROPOLIS_BUY_AREA,
	COUNT
	// END GEN INTROSPECTOR
};

enum class test_explosion_bodies {
	// GEN INTROSPECTOR enum class test_explosion_bodies
	BOMB_CASCADE_EXPLOSION,
	BOMB_CASCADE_EXPLOSION_SMALLER,
	COUNT
	// END GEN INTROSPECTOR
};

enum class test_tool_items {
	// GEN INTROSPECTOR enum class test_tool_items
	DEFUSE_KIT,

	COUNT
	// END GEN INTROSPECTOR
};

using test_flavours_map = type_map<
	type_pair<test_controlled_characters, controlled_character>,
	type_pair<test_plain_sprited_bodys, plain_sprited_body>,
	type_pair<test_shootable_weapons, shootable_weapon>,
	type_pair<test_shootable_charges, shootable_charge>,
	type_pair<test_sprite_decorations, sprite_decoration>,
	type_pair<test_complex_decorations, complex_decoration>,
	type_pair<test_wandering_pixels_decorations, wandering_pixels_decoration>,
	type_pair<test_static_lights, static_light>,
	type_pair<test_hand_explosives, hand_explosive>,
	type_pair<test_plain_missiles, plain_missile>,
	type_pair<test_finishing_traces, finishing_trace>,
	type_pair<test_container_items, container_item>,
	type_pair<test_explosive_missiles, explosive_missile>,
	type_pair<test_remnant_bodies, remnant_body>,
	type_pair<test_sound_decorations, sound_decoration>,
	type_pair<test_particles_decorations, particles_decoration>,
	type_pair<test_point_markers, point_marker>,
	type_pair<test_box_markers, box_marker>,
	type_pair<test_explosion_bodies, explosion_body>,
	type_pair<test_tool_items, tool_item>
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
auto& get_test_flavour(const all_entity_flavours& flavours, const T enum_id) {
	using E = test_flavours_map::at<T>;

	auto& into = flavours.get_for<E>();
	const auto flavour_id = to_raw_flavour_id(enum_id);
	return into[flavour_id];
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
