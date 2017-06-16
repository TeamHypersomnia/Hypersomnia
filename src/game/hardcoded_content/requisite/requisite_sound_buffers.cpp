#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/assets/assets_manager.h"

void load_requisite_sound_buffers(assets_manager& manager) {
	{
		auto& buf = manager[assets::sound_buffer_id::BUTTON_HOVER];
		buf.from_file("content/sfx/button_hover.wav");
	}

	{
		auto& buf = manager[assets::sound_buffer_id::BUTTON_CLICK];
		buf.from_file("content/sfx/button_click.wav");
	}
}