#pragma once
#include "augs/math/vec2.h"

#include "augs/templates/exception_templates.h"

#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/texture.h"
#include "augs/graphics/debug_line.h"

typedef struct __GLsync *GLsync;
typedef uint64_t GLuint64;

namespace augs {
	namespace graphics {
		class texture;
	}
	
	struct renderer_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf; 
	};

	class renderer {
		unsigned max_texture_size = static_cast<unsigned>(-1);
		debug_lines prev_logic_step_lines;

		GLuint triangle_buffer_id = 0xdeadbeef;
		GLuint special_buffer_id = 0xdeadbeef;
		GLuint imgui_elements_id = 0xdeadbeef;

		bool interpolate_debug_logic_step_lines = true;
	public:
		
		vertex_triangle_buffer triangles;
		vertex_line_buffer lines;
		special_buffer specials;

		std::size_t num_total_triangles_drawn = 0;

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
			const graphics::texture* game_world_atlas
		);

		void draw_debug_lines(
			const debug_lines& logic_step_lines,
			const debug_lines& persistent_lines,
			const debug_lines& frame_lines,

			const augs::atlas_entry line_texture, 
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

		void finish();

		void push_line(const augs::vertex_line& line) {
			lines.push_back(line);
		}

		void push_triangle(const augs::vertex_triangle& tri) {
			triangles.push_back(tri);
		}

		void push_triangles(const augs::vertex_triangle_buffer& added) {
			triangles.insert(triangles.end(), added.begin(), added.end());
		}

		void push_special_vertex_triangle(
			const augs::special s1,
			const augs::special s2,
			const augs::special s3
		) {
			specials.push_back(s1);
			specials.push_back(s2);
			specials.push_back(s3);
		}

		void clear_special_vertex_data();
		void clear_triangles();
		void clear_lines();

		void call_and_clear_lines();
		void call_and_clear_triangles();

		unsigned get_max_texture_size() const;

		std::size_t get_triangle_count() const;

		GLsync fence() const;
		bool wait_sync(GLsync, GLuint64 timeout = 0) const;

		vertex_triangle_buffer& get_triangle_buffer();
		vertex_line_buffer& get_line_buffer();
		special_buffer& get_special_buffer();
	};
}
