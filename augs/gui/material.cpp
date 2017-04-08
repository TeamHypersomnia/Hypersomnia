#include "material.h"
#include "rect.h"
#include "augs/graphics/drawers.h"
#include "game/resources/manager.h"

namespace augs {
	namespace gui {
		material::material(assets::game_image_id tex, const rgba& color) : tex(tex), color(color) {}

		material::material(const rgba& color) : tex(assets::game_image_id::BLANK), color(color) {}

		ltrb draw_clipped_rect(const material& mat, const ltrb origin, ltrb clipper, vertex_triangle_buffer& v, const bool flip) {
			const auto& manager = get_assets_manager();

			return draw_clipped_rect(v, origin, manager[mat.tex].texture_maps[texture_map_type::DIFFUSE], mat.color, clipper, flip);
		}
	}
}
