#pragma once
#include "augs/math/vec2.h"
#include "augs/templates/object_command.h"

#include "augs/graphics/renderer_settings.h"
#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/texture.h"
#include "augs/graphics/debug_line.h"
#include "augs/graphics/renderer_command_enums.h"
#include "augs/graphics/renderer_command.h"

using render_command_buffer = std::vector<augs::graphics::renderer_command>;

namespace augs {
	namespace graphics {
		class texture;

		struct renderer_command;
	}
	
	class renderer {
		debug_lines prev_logic_step_lines;

		bool interpolate_debug_logic_step_lines = true;
		renderer_settings current_settings;

		vertex_triangle_buffer triangles;
		vertex_line_buffer lines;
		special_buffer specials;

		std::size_t num_total_triangles_drawn = 0;
		std::size_t num_total_lines_drawn = 0;

	public:
		render_command_buffer commands;

		renderer(const renderer_settings&);

		void save_debug_logic_step_lines_for_interpolation(const debug_lines&);

		void draw_call_imgui(
			const graphics::texture& imgui_atlas,
			const graphics::texture* game_world_atlas,
			const graphics::texture* avatar_atlas,
			const graphics::texture* avatar_preview_atlas
		);

		void draw_debug_lines(
			const debug_lines& logic_step_lines,
			const debug_lines& persistent_lines,
			const debug_lines& frame_lines,

			const augs::atlas_entry line_texture, 
			const float interpolation_ratio
		);

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

		void clear_triangles();
		void clear_lines();

		void call_and_clear_lines();
		void call_and_clear_triangles();

		vertex_triangle_buffer& get_triangle_buffer();
		vertex_line_buffer& get_line_buffer();
		special_buffer& get_special_buffer();

		std::size_t get_triangle_count() const;

		void apply(const renderer_settings&, bool force = false);
		const renderer_settings& get_current_settings() const;

		std::size_t extract_num_total_triangles_drawn() {
			auto out = num_total_triangles_drawn;
			num_total_triangles_drawn = 0;
			return out;
		}

		std::size_t extract_num_total_lines_drawn() {
			auto out = num_total_lines_drawn;
			num_total_lines_drawn = 0;
			return out;
		}

		template <class T>
		void push_command(T&& t) {
			commands.emplace_back(graphics::renderer_command { std::forward<T>(t) });
		}

		void push_toggle(const toggle_command_type type, const bool flag) {
			push_command(toggle_command { type, flag });
		}

		void push_no_arg(const no_arg_command type) {
			push_command(type);
		}

		template <class Payload, class This>
		void push_object_command(This& t, Payload&& p) {
			graphics::renderer_command cmd;

			cmd.payload = object_command<This, Payload> {
				std::addressof(t),
				std::forward<Payload>(p)
			};

			commands.emplace_back(std::move(cmd));
		}

		template <class This, class Payload>
		void push_static_object_command(Payload&& p) {
			graphics::renderer_command cmd;

			cmd.payload = static_object_command<This, Payload> {
				std::forward<Payload>(p)
			};

			commands.emplace_back(std::move(cmd));
		}

		/* Graphics commands */
		void set_active_texture(const unsigned);
		void fullscreen_quad();

		void set_clear_color(const rgba);
		void clear_current_fbo();

		void set_blending(bool);
		void set_scissor(bool);
		void set_stencil(bool);
		void set_scissor_bounds(xywhi);

		void set_standard_blending();
		void set_overwriting_blending();
		void set_additive_blending();
		
		void call_triangles(vertex_triangle_buffer&&);
		void set_viewport(const xywhi);

		void clear_stencil();

		void start_writing_stencil();
		void start_testing_stencil();

		void stencil_positive_test();
		void stencil_reverse_test();
	};
}
