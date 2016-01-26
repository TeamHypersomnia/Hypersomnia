#pragma once
#include "graphics/pixel.h"
#include "game_framework/assets/texture.h"
#include "graphics/vertex.h"

namespace augs {
	namespace graphics {
		namespace gui {
			class rect;

			struct material {
				assets::texture_id tex;
				rgba color;
				material(assets::texture_id = assets::texture_id::BLANK, const rgba& = rgba()); 
				material(const rgba&); 
			};

			extern rects::ltrb<float> draw_clipped_rectangle(augs::texture&, rgba, rects::ltrb<float> origin, const rects::ltrb<float>* clipper, std::vector<augs::vertex_triangle>& v);
			extern rects::ltrb<float> draw_clipped_rectangle(material, rects::ltrb<float> origin, const rects::ltrb<float>* clipper, std::vector<augs::vertex_triangle>& v);
			extern rects::ltrb<float> draw_clipped_rectangle(material, rects::ltrb<float> global, const rect* clipper, std::vector<augs::vertex_triangle>& v);
		}
	}
}