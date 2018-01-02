#pragma once
#include "game/transcendental/entity_type_declaration.h"
#include "game/transcendental/cosmic_functions.h"

enum class test_scene_type {
	// GEN INTROSPECTOR enum class test_scene_type
	PARTICLE_STREAM = 1,
	FROG,
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
	SAMPLE_SUPPRESSOR,
	DAMPED_CYAN_CHARGE,
	DAMPED_CYAN_ROUND_DEFINITION,
	DAMPED_CYAN_SHELL_DEFINITION,
	CYAN_CHARGE,
	CYAN_ROUND_DEFINITION,
	CYAN_SHELL_DEFINITION,
	SAMPLE_RIFLE,
	SN_SIX_NINE,
	AMPLIFIER_ARM,
	ROUND_DEFINITION,
	ELECTRIC_MISSILE,
	SAMPLE_BACKPACK,
	CROSSHAIR,
	CROSSHAIR_RECOIL_BODY,
	ZERO_TARGET,
	URBAN_CYAN_MACHETE,
	MOTORCYCLE_FRONT,
	MOTORCYCLE_LEFT_WHEEL,
	PLAYER,
	// END GEN INTROSPECTOR
	COUNT
};

template <class C>
auto create_test_scene_entity(C& cosm, const test_scene_type id) {
	return cosmic::create_entity(cosm, static_cast<entity_type_id>(id));
}

template <class C>
auto& get_test_type(C& container, const test_scene_type id) {
	return container.get_type(static_cast<entity_type_id>(id));
}

void populate_test_scene_types(entity_types&);
