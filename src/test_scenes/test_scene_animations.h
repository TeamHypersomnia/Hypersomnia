#pragma once
#include "test_scenes/test_id_to_pool_id.h"
#include "test_scenes/test_scene_images.h"

enum class test_scene_plain_animation_id {
	// GEN INTROSPECTOR enum class test_scene_plain_animation_id
	CAST_BLINK_ANIMATION,
	WANDERING_PIXELS_ANIMATION,
	VINDICATOR_SHOOT,
	DATUM_GUN_SHOOT,

	METROPOLIS_CHARACTER_BARE,
	RESISTANCE_CHARACTER_BARE,

	METROPOLIS_CHARACTER_RIFLE,
	METROPOLIS_CHARACTER_AKIMBO,

	RESISTANCE_CHARACTER_RIFLE,
	RESISTANCE_CHARACTER_RIFLE_SHOOT,

	SILVER_TROUSERS,
	SILVER_TROUSERS_STRAFE,

	YELLOW_FISH,
	DARKBLUE_FISH,
	CYANVIOLET_FISH,
	JELLYFISH,
	DRAGON_FISH,

	WATER_SURFACE,

	COUNT
	// END GEN INTROSPECTOR
};

using test_scene_torso_animation_id = test_scene_plain_animation_id;
using test_scene_legs_animation_id = test_scene_plain_animation_id;

inline auto to_animation_id(const test_scene_plain_animation_id id) {
	return to_pool_id<assets::plain_animation_id>(id);
}
