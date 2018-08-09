#include "application/setups/editor/editor_view.h"

maybe_layer_filter editor_view::get_effective_selecting_filter() const {
	if (!viewing_filter.is_enabled) {
		return selecting_filter;
	}

	return viewing_filter.value & selecting_filter.value;
}

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

void editor_view::reset_panning() {
	panned_camera = std::nullopt;
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
