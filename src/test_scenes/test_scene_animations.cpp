#include "test_scenes/test_scenes_content.h"

#include "game/assets/ids/animation_id.h"
#include "game/assets/all_logical_assets.h"

#include "test_scenes/test_scene_images.h"

void load_test_scene_animations(all_logical_assets& anims) {
	{
		auto& anim = anims[assets::animation_id::CAST_BLINK_ANIMATION];
		anim.loop_mode = animation::loop_type::NONE;

		anim.create_frames(
			to_image_id(test_scene_image_id::CAST_BLINK_1),
			to_image_id(test_scene_image_id::CAST_BLINK_19),
			50.0f
		);
	} 
}