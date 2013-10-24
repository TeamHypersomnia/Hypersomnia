#include "stdafx.h"
#include "render_system.h"
#include "entity_system/entity.h"
#include "../resources/render_info.h"

render_system::render_system(window::glwindow& output_window) : output_window(output_window) {
	output_window.current();

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

#define AI_DEBUG_DRAW
#ifdef AI_DEBUG_DRAW
static components::transform last_camera;
#include "../components/ai_component.h"
#include "../components/physics_component.h"
#endif
void render_system::draw(rects::xywh visible_area, components::transform camera_transform, unsigned mask) {
	/* shortcut */
	typedef std::pair<components::render*, components::transform*> cached_pair;

	std::vector<cached_pair> visible_targets;

	std::vector<entity*> entities_by_mask;
	for (auto it : targets) {
		if (it->get<components::render>().mask == mask)
			entities_by_mask.push_back(it);
	}

	for (auto e : entities_by_mask) {
		auto& render = e->get<components::render>();
		if (render.model == nullptr) continue;
		auto& transform = e->get<components::transform>();
		
		/* if an entity's AABB hovers specified visible region */
		if (render.model->is_visible(visible_area + camera_transform.current.pos, transform))
			visible_targets.push_back(std::make_pair(&render, &transform));
	}

	/* we sort layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	std::stable_sort(visible_targets.begin(), visible_targets.end(), [](const cached_pair& a, const cached_pair& b) {
		return a.first->layer > b.first->layer;
	});

	for (auto e : visible_targets) {
		if (e.first->model == nullptr) continue;
		e.first->model->draw(triangles, *e.second, camera_transform.current.pos);
	}
#ifdef AI_DEBUG_DRAW
	last_camera = camera_transform.current.pos;
#endif
}

void render_system::process_entities(world&) {
}

void render_system::render() {
	glVertexPointer(2, GL_INT, sizeof(resources::vertex), triangles.data());
	glTexCoordPointer(2, GL_FLOAT, sizeof(resources::vertex), (char*) (triangles.data()) + sizeof(int) * 2);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(resources::vertex), (char*) (triangles.data()) + sizeof(int) * 2 + sizeof(float) * 2);
	glDrawArrays(GL_TRIANGLES, 0, triangles.size() * 3);

#ifdef AI_DEBUG_DRAW
	for (auto it : targets) {
		auto* ai = it->find<components::ai>();
		if (ai) {
			glColor4f(1.0, 0.0, 0.0, 1.0);
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINES);
			
			for (auto line : ai->lines) {
				glVertex2f(line.a.x*METERS_TO_PIXELSf - last_camera.current.pos.x, line.a.y*METERS_TO_PIXELSf - last_camera.current.pos.y);
				glVertex2f(line.b.x*METERS_TO_PIXELSf - last_camera.current.pos.x, line.b.y*METERS_TO_PIXELSf - last_camera.current.pos.y);
			}

			glEnd();
			glEnable(GL_TEXTURE_2D);
			ai->lines.clear();
		}
	}
#endif
	triangles.clear();
}

