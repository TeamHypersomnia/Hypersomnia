#pragma once
#include "test_scenes/test_id_to_pool_id.h"
#include "test_scenes/test_scene_images.h"

enum class test_scene_animation_id {
	// GEN INTROSPECTOR enum class test_scene_animation_id
	CAST_BLINK_ANIMATION,
	COUNT
	// END GEN INTROSPECTOR
};

struct animation;

void create_frames(
	animation& output,
	const test_scene_image_id first_frame,
	const test_scene_image_id last_frame,
	const float frame_duration_ms
);

inline auto to_animation_id(const test_scene_animation_id id) {
	return to_pool_id<assets::animation_id>(id);
}
