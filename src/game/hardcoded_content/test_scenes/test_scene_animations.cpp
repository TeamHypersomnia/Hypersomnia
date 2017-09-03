#include "game/hardcoded_content/test_scenes/test_scenes_content.h"

#include "game/assets/animation_id.h"
#include "game/assets/all_assets.h"

void load_test_scene_animations(all_logical_assets& anims) {
	{
		auto& anim = anims[assets::animation_id::CAST_BLINK_ANIMATION];
		anim.loop_mode = animation::loop_type::NONE;

		anim.create_frames(
			assets::game_image_id::CAST_BLINK_1,
			assets::game_image_id::CAST_BLINK_19,
			50.0f
		);
	} 
}