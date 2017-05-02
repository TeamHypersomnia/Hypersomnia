#pragma once
#include "augs/graphics/pixel.h"
#include "game/assets/game_image_id.h"
#include "augs/graphics/vertex.h"

namespace augs {
	namespace gui {
		struct material {
			assets::game_image_id tex;
			rgba color;
			material(assets::game_image_id = assets::game_image_id::BLANK, const rgba& = rgba());
			material(const rgba&);
		};

		ltrb draw_clipped_rect(const material&, ltrb origin, ltrb clipper, vertex_triangle_buffer& v, const bool flip = false);
		
		template <class C, class gui_element_id>
		ltrb draw_clipped_rect(
			const material& mat, 
			const ltrb origin, 
			const C context, 
			const gui_element_id& clipper, 
			vertex_triangle_buffer& v
		) {
			return draw_clipped_rect(mat, origin, context.get_tree_entry(clipper).get_absolute_clipping_rect(), v);
		}
	}
}