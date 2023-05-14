#pragma once
#include "test_scenes/test_id_to_pool_id.h"

enum class test_scene_physical_material_id {
	// GEN INTROSPECTOR enum class test_scene_physical_material_id
	WOOD,
	METAL,
	GRENADE,
	FLASHBANG,
	GLASS,
	KNIFE,
	VENT,
	CHARACTER,
	COUNT
	// END GEN INTROSPECTOR
};

inline auto to_physical_material_id(const test_scene_physical_material_id id) {
	return to_pool_id<assets::physical_material_id>(id);
}
