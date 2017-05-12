#include "all.h"

#include "game/transcendental/cosmos.h"

void set_standard_animations(assets_manager& anims) {
	{
		auto& anim = anims[assets::animation_id::CAST_BLINK_ANIMATION];
		anim.loop_mode = animation::loop_type::NONE;

		anim.create_frames(
			assets::game_image_id::CAST_BLINK_FIRST,
			assets::game_image_id::CAST_BLINK_LAST,
			50.0f
		);
	} 
	
	{
		auto& anim = anims[assets::animation_id::BLINK_ANIMATION];
		anim.loop_mode = animation::loop_type::NONE;

		anim.create_frames(
			assets::game_image_id::BLINK_FIRST,
			assets::game_image_id::BLINK_LAST,
			50.0f
		);
	}
}
