#include "material.h"
#include "rect.h"
#include "augs/texture_atlas/texture_atlas.h"
#include "augs/graphics/drawers.h"

namespace augs {
	namespace gui {
		material::material(assets::texture_id tex, const rgba& color) : tex(tex), color(color) {}

		material::material(const rgba& color) : tex(assets::texture_id::BLANK), color(color) {}

		ltrb draw_clipped_rect(const material& mat, const ltrb origin, ltrb clipper, vertex_triangle_buffer& v, const bool flip) {
			return draw_clipped_rect(v, origin, *mat.tex, mat.color, clipper, flip);
		}
	}
}
