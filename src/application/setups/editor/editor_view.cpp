#include "application/setups/editor/editor_view.h"

void editor_view::toggle_grid() {
	auto& f = show_grid;
	f = !f;
}

void editor_view::reset_zoom_at(vec2 pos) {
	if (panned_camera) {
		panned_camera->zoom = 1.f;
		panned_camera->transform.pos = pos.discard_fract();
	}
}
