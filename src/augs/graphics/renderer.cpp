#include <tuple>

#include "application/config_structs/debug_drawing_settings.h"

#include "augs/graphics/OpenGL_includes.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/fbo.h"
#include "game/messages/visibility_information.h"
#include "game/components/rigid_body_component.h"
#include "augs/templates/container_templates.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "augs/graphics/fbo.h"

#include "augs/window_framework/window.h"

#include "game/assets/assets_manager.h"
#include "augs/graphics/drawers.h"
#include "game/detail/camera_cone.h"

namespace augs {
	renderer::renderer(
		window::glwindow& parent_window,
		const debug_drawing_settings& debug
	) : 
		parent_window(parent_window),
		debug(debug)
	{
	}

	void renderer::set_as_current_impl() {

	}

	void renderer::initialize() {
		gladLoadGL();

		glEnable(GL_TEXTURE_2D); glerr;
		glEnable(GL_BLEND); glerr;

		//glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); glerr;
		glClearColor(0.0, 0.0, 0.0, 1.0); glerr;

		glGenBuffers(1, &triangle_buffer_id); glerr;
		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id); glerr;

		glEnableVertexAttribArray(int(vertex_attribute::position)); glerr;
		glEnableVertexAttribArray(int(vertex_attribute::texcoord)); glerr;
		glEnableVertexAttribArray(int(vertex_attribute::color)); glerr;

		glVertexAttribPointer(int(vertex_attribute::position), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0); glerr;
		glVertexAttribPointer(int(vertex_attribute::texcoord), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (char*)(sizeof(float) * 2)); glerr;
		glVertexAttribPointer(int(vertex_attribute::color), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), (char*)(sizeof(float) * 2 + sizeof(float) * 2)); glerr;

		glGenBuffers(1, &special_buffer_id); glerr;
		glBindBuffer(GL_ARRAY_BUFFER, special_buffer_id); glerr;

		enable_special_vertex_attribute();
		glVertexAttribPointer(int(vertex_attribute::special), sizeof(special)/sizeof(float), GL_FLOAT, GL_FALSE, sizeof(special), 0); glerr;
		disable_special_vertex_attribute();

		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id); glerr;
	}

	void renderer::initialize_fbos(const vec2i screen_size) {
		const auto sz = vec2u(screen_size);

		illuminating_smoke_fbo.create(sz.x, sz.y);
		smoke_fbo.create(sz.x, sz.y);
		light_fbo.create(sz.x, sz.y);
	}

	void renderer::enable_special_vertex_attribute() {
		glEnableVertexAttribArray(int(vertex_attribute::special)); glerr;
	}

	void renderer::disable_special_vertex_attribute() {
		glDisableVertexAttribArray(int(vertex_attribute::special)); glerr;
	}

	void renderer::clear_current_fbo() {
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	void renderer::set_active_texture(const unsigned n) {
		glActiveTexture(GL_TEXTURE0 + n);
	}

	void renderer::call_triangles() {
		if (triangles.empty()) {
			return;
		}

		if (!specials.empty()) {
			enable_special_vertex_attribute();
			glBindBuffer(GL_ARRAY_BUFFER, special_buffer_id); glerr;
			glBufferData(GL_ARRAY_BUFFER, sizeof(special) * specials.size(), specials.data(), GL_STREAM_DRAW); glerr;
		}

		call_triangles(triangles);

		if (!specials.empty()) {
			disable_special_vertex_attribute();
		}
	}

	void renderer::call_triangles(const vertex_triangle_buffer& buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id); glerr;

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_triangle) * buffer.size(), buffer.data(), GL_STREAM_DRAW); glerr;
		glDrawArrays(GL_TRIANGLES, 0, buffer.size() * 3); glerr;
		triangles_drawn_total += buffer.size();
	}

	void renderer::push_triangle(const vertex_triangle& tri) {
		triangles.push_back(tri);
	}

	void renderer::call_lines() {
		if (lines.empty()) return;

		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id); glerr;
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_line) * lines.size(), lines.data(), GL_STREAM_DRAW); glerr;
		glDrawArrays(GL_LINES, 0, lines.size() * 2); glerr;
	}

	void renderer::push_line(const vertex_line& line) {
		lines.push_back(line);
	}

	void renderer::set_viewport(const xywhi xywh) {
		glViewport(xywh.x, xywh.y, xywh.w, xywh.h);
	}
	
	void renderer::push_triangles(const vertex_triangle_buffer& added) {
		concatenate(triangles, added);
	}

	void renderer::push_special_vertex_triangle(
		const augs::special s1, 
		const augs::special s2, 
		const augs::special s3
	) {
		specials.push_back(s1);
		specials.push_back(s2);
		specials.push_back(s3);
	}
	
	void renderer::clear_special_vertex_data() {
		specials.clear();
	}

	void renderer::clear_triangles() {
		triangles.clear();
		specials.clear();
	}

	void renderer::clear_lines() {
		lines.clear();
	}

	int renderer::get_triangle_count() const {
		return triangles.size();
	}

	vertex_triangle& renderer::get_triangle(int i) {
		return triangles[i];
	}

	std::vector<vertex_triangle>& renderer::get_triangle_buffer() {
		return triangles;
	}

	void renderer::fullscreen_quad() {
		static float vertices[] = {
			1.f, 1.f,
			1.f, 0.f,
			0.f, 0.f,

			0.f, 0.f,
			0.f, 1.f,
			1.f, 1.f
		};

		glBindBuffer(GL_ARRAY_BUFFER, 0); glerr;
		glDisableVertexAttribArray(int(vertex_attribute::texcoord)); glerr;
		glDisableVertexAttribArray(int(vertex_attribute::color)); glerr;
		glVertexAttribPointer(int(vertex_attribute::position), 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), vertices); glerr;

		glDrawArrays(GL_TRIANGLES, 0, 6); glerr;

		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id); glerr;

		glEnableVertexAttribArray(int(vertex_attribute::position)); glerr;
		glEnableVertexAttribArray(int(vertex_attribute::texcoord)); glerr;
		glEnableVertexAttribArray(int(vertex_attribute::color)); glerr;

		glVertexAttribPointer(int(vertex_attribute::position), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0); glerr;
		glVertexAttribPointer(int(vertex_attribute::texcoord), 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (char*)(sizeof(float) * 2)); glerr;
		glVertexAttribPointer(int(vertex_attribute::color), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), (char*)(sizeof(float) * 2 + sizeof(float) * 2)); glerr;

		//glBegin(GL_QUADS);
		//glVertexAttrib2f(0u, 1.f, 1.f);
		//glVertexAttrib2f(0u, 1.f, 0.f);
		//glVertexAttrib2f(0u, 0.f, 0.f);
		//glVertexAttrib2f(0u, 0.f, 1.f);
		//glEnd();
	}

	void renderer::clear_logic_lines() {
		prev_logic_lines = logic_lines;
		logic_lines.lines.clear();
	}

	void renderer::clear_frame_lines() {
		frame_lines.lines.clear();
	}

	void renderer::line_channel::draw(const vec2 a, const vec2 b, const rgba col) {
		lines.push_back(debug_line(a, b, col));
	}

	void renderer::line_channel::draw_red(const vec2 a, const vec2 b) { draw(a, b, red); }
	void renderer::line_channel::draw_green(const vec2 a, const vec2 b) { draw(a, b, dark_green); }
	void renderer::line_channel::draw_blue(const vec2 a, const vec2 b) { draw(a, b, blue); }
	void renderer::line_channel::draw_yellow(const vec2 a, const vec2 b) { draw(a, b, yellow); }
	void renderer::line_channel::draw_cyan(const vec2 a, const vec2 b) { draw(a, b, cyan); }

	void renderer::draw_debug_info(
		const camera_cone camera,
		const assets::game_image_id line_texture,
		const std::vector<const_entity_handle>& target_entities,
		const double interpolation_ratio
	) {
		
		if (!debug.drawing_enabled) {
			return;
		}
		
		const auto& tex = get_assets_manager()[line_texture].texture_maps[texture_map_type::DIFFUSE];
		
		clear_triangles();

		if (debug.draw_visibility) {
			for (auto it : target_entities) {
				//auto* visibility = it.find<components::visibility>();
				//if (visibility) {
				//	for (auto& entry : visibility->full_visibility_layers) {
				//		/* shortcut */
				//		auto& request = entry.second;
				//		
				//		auto origin = it.get_logic_transform().pos;
				//
				//		for (size_t i = 0; i < request.get_num_triangles(); ++i) {
				//			auto tri = request.get_triangle(i, origin);
				//
				//			for (auto& p : tri.points) {
				//				p -= origin;
				//
				//				float expansion = 0.f;
				//				float distance_from_subject = (p - origin).length();
				//
				//				expansion = (distance_from_subject / max_visibility_expansion_distance) * visibility_expansion;
				//
				//				p *= std::min(visibility_expansion, expansion);
				//			}
				//
				//			vertex_triangle verts;
				//
				//			for (int i = 0; i < 3; ++i) {
				//
				//				auto pos = tri.points[i] - camera_transform.pos + center + origin;
				//
				//				pos.rotate(camera_transform.rotation, center);
				//				
				//				vertex new_vertex;
				//				new_vertex.texcoord.set(tex.get_u(i), tex.get_v(i));
				//				new_vertex.pos = pos;
				//				new_vertex.color = request.color;
				//				verts.vertices[i] = new_vertex;
				//			}
				//		}
				//	}
				//}
			}
		}

		call_triangles();
		clear_triangles();

		clear_lines();

		auto line_lambda = [&](const debug_line line) {
			augs::draw_line(
				lines, camera[line.a], camera[line.b], tex, line.col 
			);
		};

		if (should_interpolate_debug_lines && logic_lines.lines.size() == prev_logic_lines.lines.size()) {
			std::vector<debug_line> interpolated_logic_lines;
			interpolated_logic_lines.resize(logic_lines.lines.size());

			for (size_t i = 0; i < logic_lines.lines.size(); ++i) {
				interpolated_logic_lines[i].a = prev_logic_lines.lines[i].a.lerp(logic_lines.lines[i].a, interpolation_ratio);
				interpolated_logic_lines[i].b = prev_logic_lines.lines[i].b.lerp(logic_lines.lines[i].b, interpolation_ratio);
				interpolated_logic_lines[i].col = logic_lines.lines[i].col;
			}

			std::for_each(interpolated_logic_lines.begin(), interpolated_logic_lines.end(), line_lambda);
		}
		else
			std::for_each(logic_lines.lines.begin(), logic_lines.lines.end(), line_lambda);

		std::for_each(frame_lines.lines.begin(), frame_lines.lines.end(), line_lambda);
		std::for_each(persistent_lines.lines.begin(), persistent_lines.lines.end(), line_lambda);
		
		if (persistent_lines.lines.empty()) {
			line_timer.reset();
		}
		else if(line_timer.get<std::chrono::seconds>() > 0.4) {
			//persistent_lines.lines.erase(persistent_lines.lines.begin());
			line_timer.reset();
		}

		call_lines();
		clear_lines();

		clear_frame_lines();
	}

	void renderer::clear_geometry() {
		triangles.clear();
	}

	void renderer::bind_texture(const graphics::fbo& f) {
		glBindTexture(GL_TEXTURE_2D, f.get_texture_id()); glerr;
	}

	void renderer::bind_texture(const augs::graphics::texture& atl) {
		glBindTexture(GL_TEXTURE_2D, atl.id); glerr;
	}

	size_t renderer::get_max_texture_size() const {
		GLint tsize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tsize); glerr;
		return tsize;
	}

	void renderer::default_render(vec2 visible_world_area) {
		graphics::fbo::use_default();
		glClear(GL_COLOR_BUFFER_BIT); glerr;
		
		bind_texture(get_assets_manager()[assets::gl_texture_id::GAME_WORLD_ATLAS]);

		call_triangles();

		triangles.clear();
	}
}
