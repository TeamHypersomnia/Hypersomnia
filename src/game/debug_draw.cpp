#include "game/debug_draw.h"
#include "3rdparty/Box2D/Collision/Shapes/b2PolygonShape.h"

void debug_draw_shape(std::vector<debug_line>& lines, const rgba col, const b2PolygonShape& sh, vec2 pos) {
	const auto n = sh.GetVertexCount();

	for (int i = 0; i < n; ++i) {
		lines.emplace_back(col, pos + sh.GetVertex(i), pos + sh.GetVertex((i + 1) % n));
	}
}

void debug_draw_shape(std::vector<debug_line>& lines, const rgba col, const b2PolygonShape& sh, vec2 pos, const si_scaling si) {
	const auto n = sh.GetVertexCount();

	for (int i = 0; i < n; ++i) {
		lines.emplace_back(col, pos + si.get_pixels(sh.GetVertex(i)), pos + si.get_pixels(sh.GetVertex((i + 1) % n)));
	}
}

template <std::size_t I>
void debug_draw_verts(std::vector<debug_line>& lines, const rgba col, const std::array<vec2, I>& arr, const vec2 pos) {
	const auto n = arr.size();

	for (std::size_t i = 0; i < n; ++i) {
		lines.emplace_back(col, pos + arr[i], pos + arr[(i + 1) % n]);
	}
}

template void debug_draw_verts(std::vector<debug_line>& lines, const rgba col, const std::array<vec2, 5>& arr, const vec2 pos);
