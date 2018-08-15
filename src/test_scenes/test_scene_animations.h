#pragma once
#include "test_scenes/test_id_to_pool_id.h"
#include "test_scenes/test_scene_images.h"

enum class test_scene_plain_animation_id {
	// GEN INTROSPECTOR enum class test_scene_plain_animation_id
	CAST_BLINK,
	WANDERING_PIXELS_ANIMATION,
	VINDICATOR_SHOOT,
	DATUM_GUN_SHOOT,

	METROPOLIS_TORSO_BARE,
	METROPOLIS_TORSO_RIFLE,
	METROPOLIS_TORSO_AKIMBO,
	METROPOLIS_TORSO_HEAVY,
	METROPOLIS_TORSO_HEAVY_SHOOT,

	RESISTANCE_TORSO_BARE,
	RESISTANCE_TORSO_RIFLE,
	RESISTANCE_TORSO_RIFLE_SHOOT,

	RESISTANCE_TORSO_HEAVY_WALK,

	SILVER_TROUSERS,
	SILVER_TROUSERS_STRAFE,

	YELLOW_FISH,
	DARKBLUE_FISH,
	CYANVIOLET_FISH,
	JELLYFISH,
	DRAGON_FISH,

	FLOWER_PINK,
	FLOWER_CYAN,

	CONSOLE_LIGHT,

	WATER_SURFACE,

	SMALL_BUBBLE_LB,
	SMALL_BUBBLE_LT,
	SMALL_BUBBLE_RB,
	SMALL_BUBBLE_RT,

	MEDIUM_BUBBLE,
	BIG_BUBBLE,

	PINK_CORAL,

	BOMB,
	BOMB_ARMED,

	COUNT
	// END GEN INTROSPECTOR
};

using test_scene_torso_animation_id = test_scene_plain_animation_id;
using test_scene_legs_animation_id = test_scene_plain_animation_id;

inline auto to_animation_id(const test_scene_plain_animation_id id) {
	return to_pool_id<assets::plain_animation_id>(id);
}
