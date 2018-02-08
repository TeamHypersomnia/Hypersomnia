#pragma once
#include "augs/templates/type_map.h"

#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/cosmic_functions.h"
#include "view/viewables/game_image.h"

enum class test_static_lights {
	STRONG_LAMP = 1
};

enum class test_sprite_decorations {
	HAVE_A_PLEASANT = 1,
	STREET,
	ROAD_DIRT,
	ROAD,
	AWAKENING,
	METROPOLIS
};

enum class test_controlled_characters {
	PLAYER = 1
};

enum class test_plain_invisible_bodys {
	CROSSHAIR_RECOIL_BODY = 1
};

enum class test_plain_sprited_bodys {
	CRATE = 1,
	CYAN_SHELL_DEFINITION,
	BRICK_WALL
};

enum class test_shootable_weapons {
	SAMPLE_RIFLE = 1,
	KEK9,
	AMPLIFIER_ARM
};

enum class test_shootable_charges {
	CYAN_CHARGE = 1
};

enum class test_wandering_pixels_decorations {
	WANDERING_PIXELS = 1
};

enum class test_throwable_explosives {
	FORCE_GRENADE = 1,
	PED_GRENADE,
	INTERFERENCE_GRENADE
};

enum class test_plain_missiles {
	CYAN_ROUND_DEFINITION = 1,
	AMPLIFIER_ARM_MISSILE,
	ELECTRIC_MISSILE
};

enum class test_finishing_traces {
	CYAN_ROUND_FINISHING_TRACE = 1,
	ENERGY_BALL_FINISHING_TRACE
};

enum class test_container_items {
	SAMPLE_BACKPACK = 1,
	SAMPLE_MAGAZINE
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
inline auto to_raw_flavour_id(const T id) {
	return static_cast<raw_entity_flavour_id>(id);
}

template <class T>
inline auto to_entity_flavour_id(const T id) {
	entity_flavour_id id;
	id.type_id.set<test_flavours_map::at<T>>();
	id.raw = to_raw_flavour_id(id);
}

template <class C, class... Args>
auto create_test_scene_entity(C& cosm, const test_scene_flavour id, Args&& args...) {
	return cosmic::create_entity(cosm, to_entity_flavour_id(id), std::forward<Args>(args)...);
}

template <class C, class T>
auto& get_test_flavour(C& cosm, const T id) {
	return cosm.get_flavour<test_flavours_map::at<T>>(to_raw_flavour_id(id));
}

namespace test_flavours {
	void populate_other_types(const loaded_game_image_caches& caches, entity_flavours& flavours);
	void populate_car_types(const loaded_game_image_caches& caches, entity_flavours& flavours);
	void populate_crate_types(const loaded_game_image_caches& caches, entity_flavours& flavours);
	void populate_melee_types(const loaded_game_image_caches& caches, entity_flavours& flavours);
	void populate_backpack_types(const loaded_game_image_caches& caches, entity_flavours& flavours);
	void populate_gun_types(const loaded_game_image_caches& caches, entity_flavours& flavours);
	void populate_grenade_types(const loaded_game_image_caches& caches, entity_flavours& flavours);
	void populate_character_types(const loaded_game_image_caches& caches, entity_flavours& flavours);
}

void populate_test_scene_flavours(const loaded_game_image_caches& caches, entity_flavours& into);
