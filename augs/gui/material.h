#pragma once
#include "augs/graphics/pixel.h"
#include "game/assets/texture_id.h"
#include "augs/graphics/vertex.h"
#include "gui_element_id.h"

namespace augs {
	namespace gui {
		struct rect_leaf;

		struct material {
			assets::texture_id tex;
			rgba color;
			material(assets::texture_id = assets::texture_id::BLANK, const rgba& = rgba());
			material(const rgba&);
		};

		extern rects::ltrb<float> draw_clipped_rectangle(augs::texture&, rgba, rects::ltrb<float> origin, rects::ltrb<float> clipper, std::vector<augs::vertex_triangle>& v);
		extern rects::ltrb<float> draw_clipped_rectangle(material, rects::ltrb<float> origin, rects::ltrb<float> clipper, std::vector<augs::vertex_triangle>& v);
		extern rects::ltrb<float> draw_clipped_rectangle(material, rects::ltrb<float> global, const rect_leaf& clipper, vertex_triangle_buffer& v);
	}
}