#pragma once
#include "test_scenes/test_scene_images.h"

enum class animation_id {
	CAST_BLINK_ANIMATION
};

struct animation;

void create_frames(
	animation& output,
	const test_scene_image_id first_frame,
	const test_scene_image_id last_frame,
	const float frame_duration_ms
);

auto to_animation_id(const assets::animation_id id) {
	return id;
}
