#pragma once
#include "augs/graphics/vertex.h"
#include "augs/graphics/pixel.h"
#include "augs/math/vec2.h"
#include "game/assets/texture_id.h"

namespace augs {
	void draw_rect_with_border(vertex_triangle_buffer& v, const ltrb origin, const rgba inside_color, const rgba border_color);
	void draw_rect_with_border(vertex_triangle_buffer& v, const ltrb origin, const assets::texture_id, const rgba inside_color, const rgba border_color);
	
	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const rgba color = white);
	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const assets::texture_id, const rgba color = white);
	void draw_rect(vertex_triangle_buffer& v, const vec2 origin, const assets::texture_id, const rgba color = white);
	
	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const texture&, const rgba color);
	ltrb draw_clipped_rect(vertex_triangle_buffer& v, const ltrb origin, const texture&, const rgba colorize, ltrb clipper, const bool flip_horizontally = false);
}
