#pragma once
#include "augs/graphics/pixel.h"
#include "game/assets/texture_id.h"
#include "augs/graphics/vertex.h"

namespace augs {
	namespace gui {
		struct material {
			assets::texture_id tex;
			rgba color;
			material(assets::texture_id = assets::texture_id::BLANK, const rgba& = rgba());
			material(const rgba&);
		};

		extern rects::ltrb<float> draw_clipped_rectangle(const augs::texture&, const rgba, const rects::ltrb<float>& origin, const rects::ltrb<float>& clipper, std::vector<augs::vertex_triangle>& v);
		extern rects::ltrb<float> draw_clipped_rectangle(const material&, const rects::ltrb<float>& origin, const rects::ltrb<float>& clipper, std::vector<augs::vertex_triangle>& v);
		
		template <class C, class gui_element_id>
		rects::ltrb<float> draw_clipped_rectangle(const material& mat, const rects::ltrb<float>& origin, C context, const gui_element_id& clipper, vertex_triangle_buffer& v) {
			return draw_clipped_rectangle(mat, origin, context.get_tree_entry(clipper).get_absolute_clipping_rect(), v);
		}
	}
}