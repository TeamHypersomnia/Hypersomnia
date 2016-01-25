#pragma once
#include "graphics/pixel.h"
#include "texture_baker/texture_baker.h"
#include "graphics/vertex.h"

namespace augs {
	namespace graphics {
		namespace gui {
			extern augs::texture* null_texture;
			class rect;

			struct material {
				augs::texture* tex;
				rgba color;
				material(augs::texture* = null_texture, const rgba& = rgba()); 
				material(const rgba&); 
			};

			extern rects::ltrb<float> draw_clipped_rectangle(material, rects::ltrb<float> origin, const rects::ltrb<float>* clipper, std::vector<augs::vertex_triangle>& v);
			extern rects::ltrb<float> draw_clipped_rectangle(material, rects::ltrb<float> global, const rect* clipper, std::vector<augs::vertex_triangle>& v);
		}
	}
}