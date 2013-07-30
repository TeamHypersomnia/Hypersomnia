#include "camera_system.h"
#include <gl\GL.h>

camera_system::camera_system(render_system& raw_renderer) : raw_renderer(raw_renderer) {}

void camera_system::process_entities(world&) {
	/* we sort layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	std::sort(targets.begin(), targets.end(), [](entity* a, entity* b) {
		return a->get<components::camera>().layer > b->get<components::camera>().layer;
	});

	for (auto e = targets.begin(); e != targets.end(); ++e) {
		auto& camera = (*e)->get<components::camera>();

		if(camera.enabled) {
			glLoadIdentity();
			glOrtho(camera.ortho.l, camera.ortho.r, camera.ortho.b, camera.ortho.t, 0, 1);
			glViewport(camera.screen_rect.x, camera.screen_rect.y, camera.screen_rect.w, camera.screen_rect.h);

			raw_renderer.draw(camera.ortho, (*e)->get<components::transform>(), camera.mask);
		}
	}

	raw_renderer.render();
}