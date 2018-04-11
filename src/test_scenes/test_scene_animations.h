#pragma once
#include "test_scenes/test_id_to_pool_id.h"

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
