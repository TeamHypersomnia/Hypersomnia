#include "game/hardcoded_content/all_hardcoded_content.h"

#include "game/transcendental/cosmos.h"

void load_requisite_animations(assets_manager& anims) {
	{
		auto& anim = anims[assets::animation_id::BLINK_ANIMATION];
		anim.loop_mode = animation::loop_type::NONE;

		anim.create_frames(
			assets::game_image_id::BLINK_1,
			assets::game_image_id::BLINK_7,
			50.0f
		);
	}
}
