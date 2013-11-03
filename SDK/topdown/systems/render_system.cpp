#include "stdafx.h"
#include <algorithm>

#include "render_system.h"
#include "entity_system/entity.h"
#include "../resources/render_info.h"

#include "../components/ai_component.h"
#include "../components/physics_component.h"

render_system::render_system(window::glwindow& output_window) 
	: output_window(output_window), visibility_expansion(1.f), max_visibility_expansion_distance(1000.f), 
	draw_visibility(0),
	draw_substeering_forces(0),
	draw_steering_forces(0),
	draw_velocities(0)
{
	output_window.current();

	scene_fbo		.create(output_window.get_screen_rect().w, output_window.get_screen_rect().h);
	postprocess_fbo	.create(output_window.get_screen_rect().w, output_window.get_screen_rect().h);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

static components::transform::state last_camera;
void render_system::draw(rects::xywh visible_area, components::transform::state camera_transform, unsigned mask) {
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
		e.first->model->draw(triangles, *e.second, camera_transform.pos);
	}

	last_camera = camera_transform;
}

void render_system::process_entities(world&) {
}

void render_system::render(rects::xywh visible_area) {
	fbo::use_default();

	glVertexPointer(2, GL_FLOAT, sizeof(resources::vertex), triangles.data());
	glTexCoordPointer(2, GL_FLOAT, sizeof(resources::vertex), (char*) (triangles.data()) + sizeof(float) * 2);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(resources::vertex), (char*) (triangles.data()) + sizeof(float) * 2 + sizeof(float) * 2);
	
	scene_fbo.use();
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, triangles.size() * 3);
	
	//postprocess_fbo.use();
	//glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_TEXTURE_2D);
	
	if (draw_visibility) {
		//glColor4f(1.f, 1.f, 1.f, 1.f);
		glBegin(GL_TRIANGLES);
		for (auto it : targets) {
			auto* ai = it->find<components::ai>();
			if (ai) {
				glColor4ub(ai->visibility_color.r, ai->visibility_color.g, ai->visibility_color.b, 62);
				auto origin = it->get<components::transform>().current.pos;

				for (int i = 0; i < ai->get_num_triangles(); ++i) {
					auto& tri = ai->get_triangle(i, origin);

					for (auto& p : tri.points) {
						p -= origin;

						float expansion = 0.f;
						float distance_from_subject = (p - origin).length();

						//if (vision_length == 0.f) {
						//	expansion = max_visibility_expansion_distance;
						//}
						//else {
						//	expansion = max_visibility_expansion_distance / vision_length;
						//}

						expansion = (distance_from_subject / max_visibility_expansion_distance) * visibility_expansion;

						p *= std::min(visibility_expansion, expansion);
					}

					resources::vertex_triangle verts;

					for (int i = 0; i < 3; ++i) {
						auto pos = tri.points[i] - last_camera.pos + origin;

						glVertex2f(pos.x, pos.y);
					}

				}
			}
		}
		glEnd();
	}

	glBegin(GL_LINES);
	for (auto& line : lines) {
		glColor4ub(line.col.r, line.col.g, line.col.b, line.col.a);
		glVertex2f(line.a.x*METERS_TO_PIXELSf - last_camera.pos.x, line.a.y*METERS_TO_PIXELSf - last_camera.pos.y);
		glVertex2f(line.b.x*METERS_TO_PIXELSf - last_camera.pos.x, line.b.y*METERS_TO_PIXELSf - last_camera.pos.y);
	}
	lines.clear();
	glEnd();

	glEnable(GL_TEXTURE_2D);

	fbo::use_default();
	
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glBindTexture(GL_TEXTURE_2D, scene_fbo.getTextureId());
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glBegin(GL_QUADS);
		glTexCoord2f(0.f, 1.f); glVertex2i(visible_area.x, visible_area.y);
		glTexCoord2f(1.f, 1.f); glVertex2i(visible_area.r(), visible_area.y);
		glTexCoord2f(1.f, 0.f); glVertex2i(visible_area.r(), visible_area.b());
		glTexCoord2f(0.f, 0.f); glVertex2i(visible_area.x, visible_area.b());
	glEnd();

	triangles.clear();
}

