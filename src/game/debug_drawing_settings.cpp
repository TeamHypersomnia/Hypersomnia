#include "debug_drawing_settings.h"
#include "3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h"

debug_drawing_settings DEBUG_DRAWING;
std::vector<debug_line> DEBUG_LOGIC_STEP_LINES;
std::vector<debug_line> DEBUG_PERSISTENT_LINES;
std::vector<debug_line> DEBUG_FRAME_LINES;

class b2PolygonShape;

void debug_draw_shape(std::vector<debug_line>& lines, const rgba col, const b2PolygonShape& sh, vec2 pos) {
	const auto n = sh.GetVertexCount();

	for (int i = 0; i < n; ++i) {
		lines.emplace_back(col, pos + sh.GetVertex(i), pos + sh.GetVertex((i + 1) % n));
	}
}

void debug_draw_shape(std::vector<debug_line>& lines, const rgba col, const b2PolygonShape& in_sh, vec2 pos, const si_scaling si) {
	debug_draw_shape(lines, col, si.get_pixels(in_sh), pos);
}