#include <GL/OpenGL.h>
#undef min
#undef max

#include "renderer.h"

#include "game_framework/components/visibility_component.h"
#include "game_framework/components/physics_component.h"

#include "augs/entity_system/entity.h"

#include "fbo.h"

#include "../window_framework/window.h"

#include "game_framework/resources/manager.h"
#include "gui/gui_world.h"

namespace augs {
	renderer& renderer::get_current() {
		return window::glwindow::get_current()->glrenderer;
	}

	void renderer::initialize() {
		glEnable(GL_TEXTURE_2D); glerr;
		glEnable(GL_BLEND); glerr;
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE); glerr;
		glClearColor(0.0, 0.0, 0.0, 1.0); glerr;

		glGenBuffers(1, &triangle_buffer_id); glerr;
		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id); glerr;

		glEnableVertexAttribArray(VERTEX_ATTRIBUTES::POSITION); glerr;
		glEnableVertexAttribArray(VERTEX_ATTRIBUTES::TEXCOORD); glerr;
		glEnableVertexAttribArray(VERTEX_ATTRIBUTES::COLOR); glerr;

		glVertexAttribPointer(VERTEX_ATTRIBUTES::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0); glerr;
		glVertexAttribPointer(VERTEX_ATTRIBUTES::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (char*)(sizeof(float) * 2)); glerr;
		glVertexAttribPointer(VERTEX_ATTRIBUTES::COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), (char*)(sizeof(float) * 2 + sizeof(float) * 2)); glerr;
	}

	void renderer::clear() {
		graphics::fbo::use_default();
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void renderer::call_triangles() {
		if (triangles.empty()) return;

		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id); glerr;

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_triangle) * triangles.size(), triangles.data(), GL_STREAM_DRAW); glerr;
		glDrawArrays(GL_TRIANGLES, 0, triangles.size() * 3); glerr;
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

	void renderer::set_viewport(rects::xywh<int> xywh) {
		glViewport(xywh.x, xywh.y, xywh.w, xywh.h);
	}
	
	void renderer::push_triangles_from_gui_world(gui::gui_world& gui) {
		triangles.insert(triangles.end(), gui.triangle_buffer.begin(), gui.triangle_buffer.end());
	}

	void renderer::clear_triangles() {
		triangles.clear();
	}

	void renderer::clear_lines() {
		lines.clear();
	}

	int renderer::get_triangle_count() {
		return triangles.size();
	}

	vertex_triangle& renderer::get_triangle(int i) {
		return triangles[i];
	}

	void renderer::fullscreen_quad() {
		static float vertices[] = {
			1.f, 1.f,
			1.f, 0.f,
			0.f, 0.f,
			0.f, 1.f
		};

		glBindBuffer(GL_ARRAY_BUFFER, 0); glerr;
		glDisableVertexAttribArray(VERTEX_ATTRIBUTES::TEXCOORD); glerr;
		glDisableVertexAttribArray(VERTEX_ATTRIBUTES::COLOR); glerr;
		glVertexAttribPointer(VERTEX_ATTRIBUTES::POSITION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), vertices); glerr;

		glDrawArrays(GL_QUADS, 0, 4); glerr;

		glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer_id); glerr;

		glEnableVertexAttribArray(VERTEX_ATTRIBUTES::POSITION); glerr;
		glEnableVertexAttribArray(VERTEX_ATTRIBUTES::TEXCOORD); glerr;
		glEnableVertexAttribArray(VERTEX_ATTRIBUTES::COLOR); glerr;

		glVertexAttribPointer(VERTEX_ATTRIBUTES::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0); glerr;
		glVertexAttribPointer(VERTEX_ATTRIBUTES::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (char*)(sizeof(float) * 2)); glerr;
		glVertexAttribPointer(VERTEX_ATTRIBUTES::COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), (char*)(sizeof(float) * 2 + sizeof(float) * 2)); glerr;
	}

	void renderer::clear_logic_lines() {
		prev_logic_lines = logic_lines;
		logic_lines.lines.clear();
	}

	void renderer::clear_frame_lines() {
		frame_lines.lines.clear();
	}

	void renderer::line_channel::draw(vec2 a, vec2 b, rgba col) {
		lines.push_back(debug_line(a, b, col));
	}

	void renderer::line_channel::draw_red(vec2 a, vec2 b) { draw(a, b, red); }
	void renderer::line_channel::draw_green(vec2 a, vec2 b) { draw(a, b, green); }
	void renderer::line_channel::draw_blue(vec2 a, vec2 b) { draw(a, b, blue); }
	void renderer::line_channel::draw_yellow(vec2 a, vec2 b) { draw(a, b, yellow); }
	void renderer::line_channel::draw_cyan(vec2 a, vec2 b) { draw(a, b, cyan); }

	void renderer::draw_debug_info(vec2 visible_world_area, components::transform camera_transform, assets::texture_id tex_id, std::vector<entity_id> target_entities, double ratio) {
		if (!debug_drawing) return;
		
		auto& tex = resource_manager.find(tex_id)->tex;
		
		vec2 center = visible_world_area / 2;
		
		clear_triangles();

		if (draw_visibility) {
			for (auto it : target_entities) {
				auto* visibility = it->find<components::visibility>();
				if (visibility) {
					for (auto& entry : visibility->visibility_layers.raw) {
						/* shortcut */
						auto& request = entry.val;
						
						auto origin = it->get<components::transform>().pos;

						for (int i = 0; i < request.get_num_triangles(); ++i) {
							auto& tri = request.get_triangle(i, origin);

							for (auto& p : tri.points) {
								p -= origin;

								float expansion = 0.f;
								float distance_from_subject = (p - origin).length();

								expansion = (distance_from_subject / max_visibility_expansion_distance) * visibility_expansion;

								p *= std::min(visibility_expansion, expansion);
							}

							vertex_triangle verts;

							for (int i = 0; i < 3; ++i) {

								auto pos = tri.points[i] - camera_transform.pos + center + origin;

								pos.rotate(camera_transform.rotation, center);
								
								vertex new_vertex;
								new_vertex.texcoord.set(tex.get_u(i), tex.get_v(i));
								new_vertex.pos = pos;
								new_vertex.color = request.color;
								verts.vertices[i] = new_vertex;
							}
						}
					}
				}
			}
		}

		call_triangles();
		clear_triangles();

		clear_lines();

		auto line_lambda = [this, camera_transform, visible_world_area, center, tex](debug_line line) {
			line.a += center - camera_transform.pos;
			line.b += center - camera_transform.pos;

			line.a.rotate(camera_transform.rotation, center);
			line.b.rotate(camera_transform.rotation, center);

			vertex_line new_line;

			new_line.vertices[0].color = line.col;
			new_line.vertices[0].texcoord.set(tex.get_u(0), tex.get_v(0));
			new_line.vertices[0].pos.set(line.a.x, line.a.y);
			new_line.vertices[1].color = line.col;
			new_line.vertices[1].texcoord.set(tex.get_u(2), tex.get_v(2));
			new_line.vertices[1].pos.set(line.b.x, line.b.y);

			push_line(new_line);
		};

		if (logic_lines.lines.size() == prev_logic_lines.lines.size()) {
			std::vector<debug_line> interpolated_logic_lines;
			interpolated_logic_lines.resize(logic_lines.lines.size());

			for (int i = 0; i < logic_lines.lines.size(); ++i) {
				interpolated_logic_lines[i].a = prev_logic_lines.lines[i].a.lerp(logic_lines.lines[i].a, ratio);
				interpolated_logic_lines[i].b = prev_logic_lines.lines[i].b.lerp(logic_lines.lines[i].b, ratio);
				interpolated_logic_lines[i].col = logic_lines.lines[i].col;
			}

			std::for_each(interpolated_logic_lines.begin(), interpolated_logic_lines.end(), line_lambda);
		}
		else
			std::for_each(logic_lines.lines.begin(), logic_lines.lines.end(), line_lambda);

		std::for_each(frame_lines.lines.begin(), frame_lines.lines.end(), line_lambda);
		std::for_each(blink_lines.lines.begin(), blink_lines.lines.end(), line_lambda);
		
		if (blink_lines.lines.empty()) {
			line_timer.reset();
		}
		else if(line_timer.get<std::chrono::seconds>() > 0.4) {
			blink_lines.lines.erase(blink_lines.lines.begin());
			line_timer.reset();
		}

		call_lines();
		clear_lines();

		clear_frame_lines();
	}

	void renderer::clear_geometry() {
		triangles.clear();
	}

	void renderer::default_render(vec2 visible_world_area) {
		graphics::fbo::use_default();
		glClear(GL_COLOR_BUFFER_BIT); glerr;
		
		resource_manager.find(assets::atlas_id::GAME_WORLD_ATLAS)->bind();

		call_triangles();

		triangles.clear();
	}
}