#pragma once
#include "augs/math/vec2.h"

#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/texture.h"
#include "augs/graphics/debug_line.h"

#include "augs/math/camera_cone.h"

namespace augs {
	namespace graphics {
		class texture;
	}

	struct renderer {
		debug_lines frame_lines;
		debug_lines prev_logic_step_lines;
		
		GLuint position_buffer_id = 0xdeadbeef;
		GLuint texcoord_buffer_id = 0xdeadbeef;
		GLuint color_buffer_id = 0xdeadbeef;
		GLuint triangle_buffer_id = 0xdeadbeef;
		GLuint special_buffer_id = 0xdeadbeef;
		GLuint imgui_elements_id = 0xdeadbeef;

		vertex_triangle_buffer triangles;
		vertex_line_buffer lines;
		special_buffer specials;

		std::size_t triangles_drawn_total = 0;

		bool interpolate_debug_logic_step_lines = true;

		renderer();

		renderer(renderer&&) = delete;
		renderer& operator=(renderer&&) = delete;

		renderer(const renderer&) = delete;
		renderer& operator=(const renderer&) = delete;

		void set_active_texture(const unsigned);

		void fullscreen_quad();
		
		void save_debug_logic_step_lines_for_interpolation(const debug_lines&);

		void draw_call_imgui(
			const graphics::texture& imgui_atlas,
			const graphics::texture& game_world_atlas
		);

		void draw_debug_lines(
			const debug_lines& logic_step_lines,
			const debug_lines& persistent_lines,

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

		int get_max_texture_size() const;

		int get_triangle_count() const;
		vertex_triangle& get_triangle(const int i);

		vertex_triangle_buffer& get_triangle_buffer();
		vertex_line_buffer& get_line_buffer();
		special_buffer& get_special_buffer();
	};
}
