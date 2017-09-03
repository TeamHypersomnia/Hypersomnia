#pragma once
#include <vector>
#include <optional>

#include "augs/math/vec2.h"

#include "augs/templates/settable_as_current_mixin.h"
#include "augs/misc/timer.h"

#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/texture.h"

#include "augs/math/camera_cone.h"

namespace augs {
	namespace graphics {
		class texture;
	}

	class renderer : public settable_as_current_mixin<renderer> {
		friend class settable_as_current_base;

		bool set_as_current_impl() { return true; }
		void set_current_to_none_impl() {}
	public:
		struct debug_line {
			debug_line() = default;
			debug_line(const rgba col, const vec2 a, const vec2 b) : col(col), a(a), b(b) {}
			debug_line(const vec2 a, const vec2 b, const rgba col) : col(col), a(a), b(b) {}

			vec2 a;
			vec2 b;
			rgba col;
		};

		std::vector<debug_line> logic_lines;
		std::vector<debug_line> prev_logic_lines;
		std::vector<debug_line> frame_lines;
		std::vector<debug_line> persistent_lines;
		
		timer line_timer;

		unsigned int position_buffer_id = 0xdeadbeef;
		unsigned int texcoord_buffer_id = 0xdeadbeef;
		unsigned int color_buffer_id = 0xdeadbeef;
		unsigned int triangle_buffer_id = 0xdeadbeef;
		unsigned int special_buffer_id = 0xdeadbeef;

		unsigned int imgui_elements_id = 0xdeadbeef;

		vertex_triangle_buffer triangles;
		vertex_line_buffer lines;
		special_buffer specials;

		std::size_t triangles_drawn_total = 0;

		bool should_interpolate_debug_lines = false;

		renderer();

		renderer(renderer&&) = delete;
		renderer& operator=(renderer&&) = delete;

		renderer(const renderer&) = delete;
		renderer& operator=(const renderer&) = delete;

		void set_active_texture(const unsigned);

		void fullscreen_quad();
		
		void clear_logic_lines();
		void clear_frame_lines();
		
		void draw_call_imgui(
			const graphics::texture& imgui_atlas,
			const graphics::texture& game_world_atlas
		);

		void draw_debug_lines(
			const camera_cone,
			const augs::texture_atlas_entry line_texture, 
			const float interpolation_ratio
		);

		void set_clear_color(const rgba);
		void clear_current_fbo();

		void set_standard_blending();
		void set_additive_blending();
		
		void enable_special_vertex_attribute();
		void disable_special_vertex_attribute();
		void call_triangles();
		void call_triangles(const vertex_triangle_buffer&);
		void call_lines();
		void set_viewport(const xywhi);
		void push_line(const vertex_line&);
		void push_triangle(const vertex_triangle&);
		void push_triangles(const vertex_triangle_buffer&);
		
		void push_special_vertex_triangle(
			const augs::special, 
			const augs::special, 
			const augs::special
		);

		void clear_special_vertex_data();
		void clear_triangles();
		void clear_lines();

		void call_and_clear_lines();
		void call_and_clear_triangles();

		size_t get_max_texture_size() const;

		int get_triangle_count() const;
		vertex_triangle& get_triangle(const int i);

		vertex_triangle_buffer& get_triangle_buffer();
		vertex_line_buffer& get_line_buffer();
		special_buffer& get_special_buffer();
	};
}
