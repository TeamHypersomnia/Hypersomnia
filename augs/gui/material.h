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

		ltrb draw_clipped_rect(const material&, ltrb origin, ltrb clipper, vertex_triangle_buffer& v, const bool flip = false);
		
		template <class C, class gui_element_id>
		ltrb draw_clipped_rect(
			const material& mat, 
			const ltrb origin, 
			C context, 
			const gui_element_id& clipper, 
			vertex_triangle_buffer& v
		) {
			return draw_clipped_rect(mat, origin, context.get_tree_entry(clipper).get_absolute_clipping_rect(), v);
		}
	}
}