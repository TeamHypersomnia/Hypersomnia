#include "render_system.h"
#include <gl\glew.h>
#include "entity_system/entity.h"
#include "../render_info.h"

void render_system::add(entity* e) {
	entities_by_mask[e->get<components::render>().mask].push_back(e);
}

void render_system::remove(entity* e) {
	auto& v = entities_by_mask[e->get<components::render>().mask];
	v.erase(std::remove(v.begin(), v.end(), e), v.end());
}

void render_system::clear() {
	entities_by_mask.clear();
	processing_system::clear();
}

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

void render_system::draw(rects::xywh visible_area, components::transform camera_transform, unsigned mask) {
	/* shortcut */
	typedef std::pair<components::render*, components::transform*> cached_pair;

	std::vector<cached_pair> visible_targets;

	/* shortcut */
	auto& entities = entities_by_mask[mask];

	for (auto e = entities.begin(); e != entities.end(); ++e) {
		auto& render_info = (*e)->get<components::render>();
		if (render_info.instance == nullptr) continue;
		auto& transform = (*e)->get<components::transform>();
		
		/* if an entity's AABB hovers specified visible region */
		if (render_info.instance->is_visible(visible_area + camera_transform.current.pos, transform))
			visible_targets.push_back(std::make_pair(&render_info, &transform));
	}

	/* we sort layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	std::stable_sort(visible_targets.begin(), visible_targets.end(), [](const cached_pair& a, const cached_pair& b) {
		return a.first->layer > b.first->layer;
	});

	for (auto e = visible_targets.begin(); e != visible_targets.end(); ++e) {
		if ((*e).first->instance == nullptr) continue;
		(*e).first->instance->draw(triangles, *(*e).second, camera_transform.current.pos);
	}
}

void render_system::process_entities(world&) {
}

void render_system::render() {
	glClear(GL_COLOR_BUFFER_BIT);

	glVertexPointer(2, GL_INT, sizeof(vertex), triangles.data());
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (char*) (triangles.data()) + sizeof(int) * 2);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), (char*) (triangles.data()) + sizeof(int) * 2 + sizeof(float) * 2);
	glDrawArrays(GL_TRIANGLES, 0, triangles.size() * 3);

	output_window.swap_buffers();
	triangles.clear();
}

