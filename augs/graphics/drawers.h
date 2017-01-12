#pragma once
#include "augs/graphics/vertex.h"
#include "augs/graphics/pixel.h"
#include "augs/math/vec2.h"
#include "game/assets/texture_id.h"
#include "augs/graphics/renderable_positioning_type.h"

namespace augs {
	void draw_rect_with_border(vertex_triangle_buffer& v, const ltrb origin, const rgba inside_color, const rgba border_color);
	void draw_rect_with_border(vertex_triangle_buffer& v, const ltrb origin, const assets::texture_id, const rgba inside_color, const rgba border_color);
	
	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const rgba color = white);
	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const assets::texture_id, const rgba color = white);
	void draw_rect(vertex_triangle_buffer& v, const vec2 origin, const assets::texture_id, const rgba color = white);
	
	void draw_line(vertex_line_buffer& v, const vec2 from, const vec2 to, const texture&, const rgba color);
	void draw_dashed_line(vertex_line_buffer& v, const vec2 from, const vec2 to, const texture&, const rgba color, const float dash_length, const float dash_velocity, const float global_time);
	void draw_line(vertex_triangle_buffer& v, const vec2 from, const vec2 to, const float line_width, const texture&, const rgba color, const bool flip_horizontally = false);
	
	std::array<vec2, 4> make_sprite_points(
		const vec2 pos, 
		const vec2 size, 
		const float rotation_degrees, 
		const renderable_positioning_type positioning = renderable_positioning_type::CENTER
	);
	
	std::array<vertex_triangle, 2> make_sprite_triangles(
		const std::array<vec2, 4> points, 
		const augs::texture& considered_texture,
		const rgba col = white,
		const bool flip_horizontally = false,
		const bool flip_vertically = false);

	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const texture&, const rgba color);
	ltrb draw_clipped_rect(vertex_triangle_buffer& v, const ltrb origin, const texture&, const rgba colorize, ltrb clipper, const bool flip_horizontally = false);
}
