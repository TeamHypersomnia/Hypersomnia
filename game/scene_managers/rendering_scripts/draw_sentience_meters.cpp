#include "all.h"
#include "augs/graphics/drawers.h"

namespace rendering_scripts {
	void draw_sentience_meters(
		augs::vertex_triangle_buffer& buf,
		const components::sentience& sentience,
		vec2i left_top_position,
		const unsigned total_width,
		const unsigned vertical_bar_padding,
		const assets::texture_id health_icon,
		const assets::texture_id personal_electricity_icon,
		const assets::texture_id consciousness_icon
	) {
		{
			augs::draw_rect(buf, left_top_position, health_icon);

			left_top_position.y += (*health_icon).get_size().y + vertical_bar_padding;
		}
		
		{
			augs::draw_rect(buf, left_top_position, personal_electricity_icon);

			left_top_position.y += (*personal_electricity_icon).get_size().y + vertical_bar_padding;
		}

		{
			augs::draw_rect(buf, left_top_position, consciousness_icon);
		}
	}
}
