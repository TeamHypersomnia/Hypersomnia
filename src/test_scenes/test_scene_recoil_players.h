#pragma once
#include "test_scenes/test_id_to_pool_id.h"

enum class test_scene_recoil_id {
	// GEN INTROSPECTOR enum class test_scene_recoil_id
	GENERIC,
	COUNT
	// END GEN INTROSPECTOR
};

inline auto to_recoil_id(const test_scene_recoil_id id) {
	return to_pool_id<assets::recoil_player_id>(id);
}
