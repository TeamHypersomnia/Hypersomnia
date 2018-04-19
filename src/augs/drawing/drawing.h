#pragma once
#include "augs/graphics/vertex.h"
#include "augs/graphics/rgba.h"

#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "augs/math/snapping_grid.h"
#include "augs/math/camera_cone.h"

#include "augs/texture_atlas/texture_atlas_entry.h"
#include "augs/drawing/flip.h"
#include "augs/drawing/border.h"

struct grid_render_settings;

namespace augs {
	std::array<vec2, 4> make_sprite_points(
		const vec2 pos,
		const vec2i size,
		const float rotation_degrees
	);

	std::array<vertex_triangle, 2> make_sprite_triangles(
		const texture_atlas_entry considered_texture,
		const std::array<vec2, 4> points,
		const rgba col = white,
		const flip_flags = {}
	);

	struct drawer {
		vertex_triangle_buffer& output_buffer;

		operator vertex_triangle_buffer&() const {
			return output_buffer;
		}

		using self = const drawer&;

		self color_overlay(
			const texture_atlas_entry,
			const vec2i screen_size,
			const rgba color
		) const;

		self aabb(
			const texture_atlas_entry,
			const ltrb origin,
			const rgba color = white,
			const flip_flags = {}
		) const;

		self aabb_lt(
			const texture_atlas_entry,
			const vec2 left_top,
			const rgba color = white
		) const;

		self aabb_centered(
			const texture_atlas_entry,
			const vec2 center,
			const rgba color
		) const;

		self aabb_centered(
			const texture_atlas_entry,
			const vec2 center,
			const vec2i size,
			const rgba color
		) const;

		self aabb_lt_clipped(
			const texture_atlas_entry,
			const vec2 left_top,
			ltrb clipper,
			const rgba color,
			const flip_flags flip = {}
		) const;

		self aabb_clipped(
			const texture_atlas_entry,
			const ltrb origin,
			ltrb clipper,
			const rgba colorize,
			const flip_flags flip = {}
		) const;

		template <class C, class I>
		self gui_box_center_tex(
			const texture_atlas_entry tex,
			const C context,
			const I id,
			const rgba colorize = white,
			const vec2i offset = { 0, 0 }
		) const {
			const auto rects = context.get_drawing_rects(id);
			const auto tex_size = tex.get_original_size();

			auto origin = ltrbi(vec2i(0, 0), tex_size).place_in_center_of(rects.absolute);
			origin.set_position(origin.get_position() + offset);

			return aabb_clipped(tex, origin, rects.clipper, colorize);
		}

		template <class C, class I>
		self gui_box_stretch_tex(
			const texture_atlas_entry tex,
			const C context,
			const I id,
			const rgba colorize = white
		) const {
			const auto rects = context.get_drawing_rects(id);

			return aabb_clipped(tex, rects.absolute, rects.clipper, colorize);
		}

		self border(
			const texture_atlas_entry,
			ltrb origin,
			const rgba color,
			const border_input = border_input()
		) const;

		self aabb_with_border(
			const texture_atlas_entry,
			const ltrb origin,
			const rgba inside_color,
			const rgba border_color,
			const border_input = border_input()
		) const;

		self rect(
			const texture_atlas_entry,
			const transform center,
			const vec2 size,
			const rgba color = white
		) const;

		self rectangular_clock(
			const texture_atlas_entry,
			const ltrb origin,
			const rgba color,
			const float ratio
		) const;

		template <class M, class I>
		self cursor(
			const M& manager,
			const I id,
			const vec2 origin,
			const rgba color = white
		) const {
			return aabb_lt(manager.at(id), origin + get_cursor_offset(id), color);
		}

		self line(
			const texture_atlas_entry,
			const vec2 from,
			const vec2 to,
			const float line_width, 
			const rgba color, 
			const flip_flags flip = {}
		) const;

		self grid(
			texture_atlas_entry,
			vec2i screen_size,
			unsigned unit,
			camera_cone,
			const grid_render_settings&
		) const;

		template <class... Args>
		self push(Args&&... args) const {
			output_buffer.emplace_back(std::forward<Args>(args)...);
			return *this;
		}
	};

	struct drawer_with_default : public drawer {
		using base = drawer;
		using self = const drawer_with_default&;

		const texture_atlas_entry default_texture;

		drawer_with_default(
			vertex_triangle_buffer& output_buffer,
			const texture_atlas_entry default_texture
		) :
			base({ output_buffer }),
			default_texture(default_texture)
		{}
		
		using drawer::color_overlay;
		using drawer::aabb;
		using drawer::aabb_lt;
		using drawer::aabb_centered;
		using drawer::aabb_clipped;
		using drawer::border;
		using drawer::aabb_with_border;
		using drawer::rect;
		using drawer::rectangular_clock;
		using drawer::line;
		using drawer::grid;

		template <class... Args>
		self color_overlay(Args&&... args) const {
			base::color_overlay(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self aabb(Args&&... args) const {
			base::aabb(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self aabb_lt(Args&&... args) const {
			base::aabb_lt(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self aabb_centered(Args&&... args) const {
			base::aabb_centered(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self aabb_clipped(Args&&... args) const {
			base::aabb_clipped(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self border(Args&&... args) const {
			base::border(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self aabb_with_border(Args&&... args) const {
			base::aabb_with_border(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self gui_box_stretch_tex(Args&&... args) const {
			base::gui_box_stretch_tex(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self rect(Args&&... args) const {
			base::rect(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self rectangular_clock(Args&&... args) const {
			base::rectangular_clock(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self line(Args&&... args) const {
			base::line(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self grid(Args&&... args) const {
			base::grid(default_texture, std::forward<Args>(args)...);
			return *this;
		}
	};

	struct line_drawer {
		using self = const line_drawer&;

		vertex_line_buffer& output_buffer;

		self line(
			texture_atlas_entry,
			vec2 from,
			vec2 to,
			rgba color
		) const;

		self dashed_line(
			texture_atlas_entry,
			vec2 from,
			vec2 to,
			rgba color,
			float dash_length,
			float dash_velocity,
			double global_time_seconds
		) const;
	};

	struct line_drawer_with_default : public line_drawer {
		using base = line_drawer;
		using self = const line_drawer_with_default&;

		const texture_atlas_entry default_texture;

		line_drawer_with_default(
			vertex_line_buffer& output_buffer,
			const texture_atlas_entry default_texture
		) : 
			base({ output_buffer }), 
			default_texture(default_texture) 
		{}

		using line_drawer::line;
		using line_drawer::dashed_line;

		template <class... Args>
		self line(Args&&... args) const {
			base::line(default_texture, std::forward<Args>(args)...);
			return *this;
		}

		template <class... Args>
		self dashed_line(Args&&... args) const {
			base::dashed_line(default_texture, std::forward<Args>(args)...);
			return *this;
		}
	};
}
