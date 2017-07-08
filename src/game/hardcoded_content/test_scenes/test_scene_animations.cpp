#if BUILD_TEST_SCENES
#include "game/hardcoded_content/all_hardcoded_content.h"

#include "game/transcendental/cosmos.h"

void load_test_scene_animations(assets_manager& anims) {
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
#endif