#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_animations.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/all_logical_assets.h"

#include "test_scenes/test_scene_images.h"
#include "augs/string/format_enum.h"

void create_frames(
	animation& anim,
	const test_scene_image_id first_frame,
	const test_scene_image_id last_frame,
	const float frame_duration_ms
) {
	for (auto i = int(first_frame); i < int(last_frame); ++i) {
		animation_frame frame;
		frame.duration_milliseconds = frame_duration_ms;
		frame.image_id = to_image_id(test_scene_image_id(i));

		anim.frames.push_back(frame);
	}
}

void load_test_scene_animations(animations_pool& anims) {
	{
		animation anim;
		anim.loop_mode = animation::loop_type::NONE;

		create_frames(
			anim,
			test_scene_image_id::CAST_BLINK_1,
			test_scene_image_id::CAST_BLINK_19,
			50.0f
		);

		const auto test_id = test_scene_animation_id::CAST_BLINK_ANIMATION;
		anim.name = format_enum(test_id);

		const auto id = to_animation_id(test_id);
		const auto new_allocation = anims.allocate(std::move(anim));

		ensure_eq(new_allocation.key, id);
	} 
}