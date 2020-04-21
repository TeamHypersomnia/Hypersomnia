#pragma once
#include "view/viewables/viewables_loading_type.h"

struct default_setup_settings {
	static constexpr bool handles_window_input = false;
	static constexpr bool has_additional_highlights = false;
};

struct game_frame_buffer;
struct renderer_backend_result;