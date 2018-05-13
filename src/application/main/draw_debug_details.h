#include "augs/math/declare_math.h"
#include "game/transcendental/entity_handle_declaration.h"

struct session_profiler;
struct cosmic_profiler;
struct audiovisual_profiler;
struct frame_profiler;
struct atlas_profiler;

namespace augs {
	struct drawer;
	struct baked_font;
}

void draw_debug_details(
	const augs::drawer output,
	const augs::baked_font& gui_font,
	const vec2i screen_size,
	const const_entity_handle viewed_character,
	const frame_profiler& frame_performance,
	const viewables_streaming_profiler& streaming_performance,
	const atlas_profiler& general_atlas_performance,
	const session_profiler& session_performance,
	const audiovisual_profiler& audiovisual_performance
);