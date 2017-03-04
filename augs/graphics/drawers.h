#pragma once
#include "augs/graphics/vertex.h"
#include "augs/graphics/pixel.h"
#include "augs/math/vec2.h"
#include "game/assets/game_image_id.h"
#include "augs/graphics/renderable_positioning_type.h"

namespace augs {
	void draw_rect_with_border(
		vertex_triangle_buffer& v, 
		const ltrb origin, 
		const rgba inside_color, 
		const rgba border_color,
		const int border_spacing = 0
	);

	void draw_rect_with_border(
		vertex_triangle_buffer& v, 
		const ltrb origin, 
		const assets::game_image_id, 
		const rgba inside_color, 
		const rgba border_color,
		const int border_spacing = 0
	);
	
	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const rgba color = white);
	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const assets::game_image_id, const rgba color = white);
	void draw_rect(vertex_triangle_buffer& v, const vec2 origin, const assets::game_image_id, const rgba color = white);
	
	void draw_line(vertex_line_buffer& v, const vec2 from, const vec2 to, const texture_atlas_entry&, const rgba color);
	void draw_dashed_line(vertex_line_buffer& v, const vec2 from, const vec2 to, const texture_atlas_entry&, const rgba color, const float dash_length, const float dash_velocity, const float global_time_seconds);
	void draw_line(vertex_triangle_buffer& v, const vec2 from, const vec2 to, const float line_width, const texture_atlas_entry&, const rgba color, const bool flip_horizontally = false);
	
	std::array<vec2, 4> make_sprite_points(
		const vec2 pos, 
		const vec2 size, 
		const float rotation_degrees, 
		const renderable_positioning_type positioning = renderable_positioning_type::CENTER
	);
	
	std::array<vertex_triangle, 2> make_sprite_triangles(
		const std::array<vec2, 4> points, 
		const augs::texture_atlas_entry& considered_texture,
		const rgba col = white,
		const bool flip_horizontally = false,
		const bool flip_vertically = false
	);

	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const texture_atlas_entry&, const rgba color);
	ltrb draw_clipped_rect(vertex_triangle_buffer& v, const ltrb origin, const texture_atlas_entry&, const rgba colorize, ltrb clipper, const bool flip_horizontally = false);

	void draw_rectangle_clock(
		vertex_triangle_buffer& v,
		const float ratio,
		const ltrb origin, 
		const rgba color = white
	);
}
