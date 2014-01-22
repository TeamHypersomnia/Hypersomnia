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

	scene_fbo.create(output_window.get_screen_rect().w, output_window.get_screen_rect().h);
	postprocess_fbo.create(output_window.get_screen_rect().w, output_window.get_screen_rect().h);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);

	//glGenBuffers(1, &position_buffer);
	//glGenBuffers(1, &texcoord_buffer);
	//glGenBuffers(1, &color_buffer);

	glGenBuffers(1, &triangle_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);

	glEnableVertexAttribArray(VERTEX_ATTRIBUTES::POSITION);
	glEnableVertexAttribArray(VERTEX_ATTRIBUTES::TEXCOORD);
	glEnableVertexAttribArray(VERTEX_ATTRIBUTES::COLOR);

	glVertexAttribPointer(VERTEX_ATTRIBUTES::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(resources::vertex), 0);
	glVertexAttribPointer(VERTEX_ATTRIBUTES::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(resources::vertex), (char*)(sizeof(float) * 2));
	glVertexAttribPointer(VERTEX_ATTRIBUTES::COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(resources::vertex), (char*) (sizeof(float) * 2 + sizeof(float) * 2));
	//glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY);
}

void render_system::generate_triangles(rects::xywh visible_area, components::transform::state camera_transform, int mask) {
	/* shortcut */
	typedef std::pair<components::render*, components::transform::state*> cached_pair;

	std::vector<cached_pair> visible_targets;

	std::vector<entity*> entities_by_mask;
	for (auto it : targets) {
		if (it->get<components::render>().mask == mask)
			entities_by_mask.push_back(it);
	}
	 
	for (auto e : entities_by_mask) {
		auto& render = e->get<components::render>();
		if (render.model == nullptr) continue;
		auto& transform = e->get<components::transform>().current;

		/* if an entity's AABB hovers specified visible region */
		if (render.model->is_visible(visible_area + camera_transform.pos, transform))
			visible_targets.push_back(std::make_pair(&render, &transform));
	}

	/* we sort layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	std::stable_sort(visible_targets.begin(), visible_targets.end(), [](const cached_pair& a, const cached_pair& b) {
		return a.first->layer > b.first->layer;
	});

	for (auto e : visible_targets) {
		if (e.first->model == nullptr) continue;
		e.first->model->draw(triangles, *e.second, camera_transform.pos, e.first);
	}
}

void render_system::process_entities(world&) {
}

void render_system::call_triangles() {
	//if (last_bound_buffer_location != triangles.data()) {
	//	last_bound_buffer_location = triangles.data();
	//glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(resources::vertex_triangle) * triangles.size(), triangles.data(), GL_STREAM_DRAW);

		//glBindBuffer(GL_ARRAY_BUFFER, texcoord_buffer);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(resources::vertex) * triangles.size(), triangles.data(), GL_STREAM_DRAW);
		//glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(resources::vertex) * triangles.size(), triangles.data(), GL_STREAM_DRAW);

		//glVertexPointer(2, GL_FLOAT, sizeof(resources::vertex), last_bound_buffer_location);
		//glTexCoordPointer(2, GL_FLOAT, sizeof(resources::vertex), (char*) (last_bound_buffer_location) +sizeof(float) * 2);
		//glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(resources::vertex), (char*) (last_bound_buffer_location) +sizeof(float) * 2 + sizeof(float) * 2);
	//}

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

void render_system::draw_debug_info(components::transform::state camera_transform) {
	if (draw_visibility) {
		//glColor4f(1.f, 1.f, 1.f, 1.f);
		glBegin(GL_TRIANGLES);
		for (auto it : targets) {
			auto* visibility = it->find<components::visibility>();
			if (visibility) {
				for (auto& entry : visibility->visibility_layers.raw) {
					/* shortcut */
					auto& request = entry.val;

					glColor4ub(request.color.r, request.color.g, request.color.b, request.color.a / 2);
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
							auto pos = tri.points[i] - camera_transform.pos + origin;

							glVertex2f(pos.x, pos.y);
						}
					}
				}
			}
		}
		glEnd();
	}

	glBegin(GL_LINES);
	for (auto& line : lines) {
		glColor4ub(line.col.r, line.col.g, line.col.b, line.col.a);
		glVertex2f(line.a.x - camera_transform.pos.x, line.a.y - camera_transform.pos.y);
		glVertex2f(line.b.x - camera_transform.pos.x, line.b.y - camera_transform.pos.y);
	}
	for (auto& line : manually_cleared_lines) {
		glColor4ub(line.col.r, line.col.g, line.col.b, line.col.a);
		glVertex2f(line.a.x - camera_transform.pos.x, line.a.y - camera_transform.pos.y);
		glVertex2f(line.b.x - camera_transform.pos.x, line.b.y - camera_transform.pos.y);
	}
	for (auto& line : non_cleared_lines) {
		glColor4ub(line.col.r, line.col.g, line.col.b, line.col.a);
		glVertex2f(line.a.x - camera_transform.pos.x, line.a.y - camera_transform.pos.y);
		glVertex2f(line.b.x - camera_transform.pos.x, line.b.y - camera_transform.pos.y);
	}
	for (auto& line : global_debug) {
		glColor4ub(line.col.r, line.col.g, line.col.b, line.col.a);
		glVertex2f(line.a.x - camera_transform.pos.x, line.a.y - camera_transform.pos.y);
		glVertex2f(line.b.x - camera_transform.pos.x, line.b.y - camera_transform.pos.y);
	}
	glEnd();
}

void render_system::cleanup() {
	lines.clear();
	triangles.clear();
}

void render_system::default_render(rects::xywh visible_area) {
	//scene_fbo.use();
	augmentations::graphics::fbo::use_default();
	glClear(GL_COLOR_BUFFER_BIT);

	call_triangles();

	triangles.clear();
}

