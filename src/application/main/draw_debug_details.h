#include "augs/math/declare_math.h"
#include "game/transcendental/entity_handle_declaration.h"

class session_profiler;
class cosmic_profiler;

namespace augs {
	struct drawer;
	struct baked_font;
}

void draw_debug_details(
	const augs::drawer output,
	const augs::baked_font& gui_font,
	const vec2i screen_size,
	const const_entity_handle viewed_character,
	const session_profiler& session_performance,
	const cosmic_profiler& cosmos_performance
);