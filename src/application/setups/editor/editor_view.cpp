#include "application/setups/editor/editor_view.h"

void editor_view::toggle_grid() {
	auto& f = show_grid;
	f = !f;
}

void editor_view::toggle_ignore_groups() {
	auto& f = ignore_groups;
	f = !f;
}

void editor_view::toggle_snapping() {
	auto& f = snapping_enabled;
	f = !f;
}

void editor_view::reset_zoom() {
	if (panned_camera) {
		panned_camera->zoom = 1.f;
	}
}

void editor_view::reset_zoom_at(vec2 pos) {
	if (panned_camera) {
		panned_camera->zoom = 1.f;
		panned_camera->transform.pos = pos.discard_fract();
	}
}

void editor_view::center_at(vec2 pos) {
	if (!panned_camera) {
		panned_camera = camera_eye();
	}

	panned_camera->transform.pos = pos.discard_fract();
}

void editor_view::toggle_flavour_rect_selection() {
	switch (rect_select_mode) {
		case editor_rect_select_type::SAME_FLAVOUR: 
			rect_select_mode = editor_rect_select_type::EVERYTHING; 
		break;

		default: 
			rect_select_mode = editor_rect_select_type::SAME_FLAVOUR;
		break;
	}
}
