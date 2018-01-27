#pragma once
#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/cosmic_functions.h"

enum class test_scene_type {
	// GEN INTROSPECTOR enum class test_scene_type
	PARTICLE_STREAM = 1,
	WANDERING_PIXELS,
	LAMP,
	HAVE_A_PLEASANT,
	GROUND,
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
#if TODO
	ELECTRIC_MISSILE,
#endif

	SAMPLE_BACKPACK,
	CROSSHAIR,
	CROSSHAIR_RECOIL_BODY,
	ZERO_TARGET,
	URBAN_CYAN_MACHETE,
	PLAYER,
	CYAN_ROUND_FINISHING_TRACE,
	ENERGY_BALL_FINISHING_TRACE,
	// END GEN INTROSPECTOR
	COUNT
};

inline auto to_entity_type_id(const test_scene_type id) {
	return static_cast<entity_type_id>(static_cast<unsigned>(id));
}

template <class C>
auto create_test_scene_entity(C& cosm, const test_scene_type id) {
	return cosmic::create_entity(cosm, to_entity_type_id(id));
}

template <class C>
auto& get_test_type(C& container, const test_scene_type id) {
	return container.get_type(to_entity_type_id(id));
}

namespace test_types {
	void populate_other_types(const all_logical_assets& logicals, entity_types& types);
	void populate_car_types(const all_logical_assets& logicals, entity_types& types);
	void populate_crate_types(const all_logical_assets& logicals, entity_types& types);
	void populate_melee_types(const all_logical_assets& logicals, entity_types& types);
	void populate_backpack_types(const all_logical_assets& logicals, entity_types& types);
	void populate_gun_types(const all_logical_assets& logicals, entity_types& types);
	void populate_grenade_types(const all_logical_assets& logicals, entity_types& types);
	void populate_character_types(const all_logical_assets& logicals, entity_types& types);
}

void populate_test_scene_types(const all_logical_assets& logicals, entity_types& into);
