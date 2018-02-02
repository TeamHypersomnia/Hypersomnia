#pragma once
#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/cosmic_functions.h"
#include "view/viewables/game_image.h"

enum class test_scene_flavour {
	// GEN INTROSPECTOR enum class test_scene_flavour
	WANDERING_PIXELS = 1,
	STRONG_LAMP,
	HAVE_A_PLEASANT,
	STREET,
	ROAD_DIRT,
	ROAD,
	AWAKENING,
	METROPOLIS,
	CRATE,
	BRICK_WALL,
	TRUCK_FRONT,
	TRUCK_INTERIOR,
	TRUCK_LEFT_WHEEL,
	TRUCK_ENGINE_BODY,

	FORCE_GRENADE,
	PED_GRENADE,
	INTERFERENCE_GRENADE,

	SAMPLE_MAGAZINE,
	CYAN_CHARGE,
	CYAN_ROUND_DEFINITION,
	CYAN_SHELL_DEFINITION,
	SAMPLE_RIFLE,
	KEK9,
	AMPLIFIER_ARM,
	AMPLIFIER_ARM_MISSILE,
	ELECTRIC_MISSILE,

	SAMPLE_BACKPACK,
	CROSSHAIR_RECOIL_BODY,
	ZERO_TARGET,
	URBAN_CYAN_MACHETE,
	PLAYER,
	CYAN_ROUND_FINISHING_TRACE,
	ENERGY_BALL_FINISHING_TRACE,
	// END GEN INTROSPECTOR
	COUNT
};

inline auto to_entity_flavour_id(const test_scene_flavour id) {
	return static_cast<entity_flavour_id>(static_cast<unsigned>(id));
}

template <class C>
auto create_test_scene_entity(C& cosm, const test_scene_flavour id) {
	return cosmic::create_entity(cosm, to_entity_flavour_id(id));
}

template <class entity_type, class C>
auto& get_test_flavour(C& cosm, const test_scene_flavour id) {
	return cosm.get_flavour<entity_type>(to_entity_flavour_id<entity_type>(id));
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
