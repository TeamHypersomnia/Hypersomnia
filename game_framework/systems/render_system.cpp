#include "stdafx.h"
#include <algorithm>

#include "render_system.h"
#include "entity_system/entity.h"
#include "../resources/render_info.h"

#include "../components/visibility_component.h"
#include "../components/physics_component.h"

render_system::render_system(window::glwindow& output_window)
	: output_window(output_window), visibility_expansion(1.f), max_visibility_expansion_distance(1000.f),
	draw_visibility(0),
	draw_substeering_forces(0),
	draw_steering_forces(0),
	draw_velocities(0),
	draw_avoidance_info(0),
	draw_wandering_info(0),
	debug_drawing(0),
	draw_weapon_info(0),
	last_bound_buffer_location(nullptr)
{
	output_window.current();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);

	glGenBuffers(1, &triangle_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);

	glEnableVertexAttribArray(VERTEX_ATTRIBUTES::POSITION);
	glEnableVertexAttribArray(VERTEX_ATTRIBUTES::TEXCOORD);
	glEnableVertexAttribArray(VERTEX_ATTRIBUTES::COLOR);

	glVertexAttribPointer(VERTEX_ATTRIBUTES::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(resources::vertex), 0);
	glVertexAttribPointer(VERTEX_ATTRIBUTES::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(resources::vertex), (char*)(sizeof(float) * 2));
	glVertexAttribPointer(VERTEX_ATTRIBUTES::COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(resources::vertex), (char*) (sizeof(float) * 2 + sizeof(float) * 2));
}

void render_system::generate_layers(int mask) {
	layers.clear();

	/* shortcut */
	std::vector<entity*> entities_by_mask;
	for (auto it : targets) {
		if (it->get<components::render>().mask == mask)
			entities_by_mask.push_back(it);
	}

	for (auto it : entities_by_mask) {
		auto layer = it->get<components::render>().layer;
		
		if (layer >= layers.size()) 
			layers.resize(layer+1);

		layers[layer].push_back(it);
	}
}

void render_system::draw_layer(resources::renderable::draw_input& in, int layer) {
	auto in_camera_transform = in.camera_transform;
	auto in_always_visible = in.always_visible;
	
	if (layer < layers.size() && !layers[layer].empty()) {
		for (auto e : layers[layer]) {
			auto& render = e->get<components::render>();
			if (render.model == nullptr) continue;

			in.transform = e->get<components::transform>().current;
			in.additional_info = &render;

			in.camera_transform = render.absolute_transform ? components::transform::state<>() : in_camera_transform;
			in.always_visible = render.absolute_transform ? true : in_always_visible;

			render.model->draw(in);
		}
	}

	in.camera_transform = in_camera_transform;
	in.always_visible = in_always_visible;
}

void render_system::generate_triangles(resources::renderable::draw_input& in, int mask) {
	generate_layers(mask);

	for (int i = 0; i < layers.size(); ++i)
		draw_layer(in, layers.size()-i-1);
}

void render_system::process_entities(world&) {
}

void render_system::call_triangles() {
	glBufferData(GL_ARRAY_BUFFER, sizeof(resources::vertex_triangle) * triangles.size(), triangles.data(), GL_STREAM_DRAW);
	//std::cout << "triangles:" << triangles.size() << std::endl;
	glDrawArrays(GL_TRIANGLES, 0, triangles.size() * 3);
}

void render_system::push_triangle(const resources::vertex_triangle& tri) {
	triangles.push_back(tri);
}

void render_system::clear_triangles() {
	triangles.clear();
}

int render_system::get_triangle_count() {
	return triangles.size();
}

resources::vertex_triangle& render_system::get_triangle(int i) {
	return triangles[i];
}

void render_system::draw_debug_info(vec2<> visible_area, components::transform::state<> camera_transform, augs::texture_baker::texture* tex) {
	vec2<> center = visible_area / 2;

	if (draw_visibility) {
		glBegin(GL_TRIANGLES);
		for (auto it : targets) {
			auto* visibility = it->find<components::visibility>();
			if (visibility) {
				for (auto& entry : visibility->visibility_layers.raw) {
					/* shortcut */
					auto& request = entry.val;


					glVertexAttrib4f(VERTEX_ATTRIBUTES::COLOR, request.color.r / 255.f, request.color.g / 255.f, request.color.b / 255.f, request.color.a / 2 / 255.f);
					auto origin = it->get<components::transform>().current.pos;

					for (int i = 0; i < request.get_num_triangles(); ++i) {
						auto& tri = request.get_triangle(i, origin);

						for (auto& p : tri.points) {
							p -= origin;

							float expansion = 0.f;
							float distance_from_subject = (p - origin).length();

							expansion = (distance_from_subject / max_visibility_expansion_distance) * visibility_expansion;

							p *= std::min(visibility_expansion, expansion);
						}

						resources::vertex_triangle verts;

						for (int i = 0; i < 3; ++i) {

							auto pos = tri.points[i] - camera_transform.pos + center + origin;

							pos.rotate(camera_transform.rotation, center);
							
							if (tex) 
								glVertexAttrib2f(VERTEX_ATTRIBUTES::TEXCOORD, tex->get_u(i), tex->get_v(i));
							
							glVertexAttrib2f(VERTEX_ATTRIBUTES::POSITION, pos.x, pos.y);
						}
					}
				}
			}
		}
		glEnd();
	}

	glBegin(GL_LINES);
	
	auto line_lambda = [camera_transform, visible_area, center, tex](debug_line line) {
		line.a += center - camera_transform.pos;
		line.b += center - camera_transform.pos;

		line.a.rotate(camera_transform.rotation, center);
		line.b.rotate(camera_transform.rotation, center);
		glVertexAttrib4f(VERTEX_ATTRIBUTES::COLOR, line.col.r / 255.f, line.col.g / 255.f, line.col.b / 255.f, line.col.a / 255.f);
		if (tex) glVertexAttrib2f(VERTEX_ATTRIBUTES::TEXCOORD, tex->get_u(0), tex->get_v(0));
		glVertexAttrib2f(VERTEX_ATTRIBUTES::POSITION, line.a.x, line.a.y);
		if (tex) glVertexAttrib2f(VERTEX_ATTRIBUTES::TEXCOORD, tex->get_u(2), tex->get_v(2));
		glVertexAttrib2f(VERTEX_ATTRIBUTES::POSITION, line.b.x, line.b.y);
	};
	
	std::for_each(lines.begin(), lines.end(), line_lambda);

	for (int i = 0; i < 20; ++i) {
		std::for_each(lines_channels[i].begin(), lines_channels[i].end(), line_lambda);
	}

	std::for_each(manually_cleared_lines.begin(), manually_cleared_lines.end(), line_lambda);
	std::for_each(non_cleared_lines.begin(), non_cleared_lines.end(), line_lambda);

	glEnd();
}

void render_system::cleanup() {
	lines.clear();
	triangles.clear();
}

void render_system::default_render(vec2<> visible_area) {
	augs::graphics::fbo::use_default();
	glClear(GL_COLOR_BUFFER_BIT);

	call_triangles();

	triangles.clear();
}

