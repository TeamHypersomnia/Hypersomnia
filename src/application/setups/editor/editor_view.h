#pragma once
#include "augs/math/camera_cone.h"
#include "augs/math/snapping_grid.h"
#include "application/setups/editor/selector/editor_rect_select_type.h"

struct editor_view {
	// GEN INTROSPECTOR struct editor_view
	snapping_grid grid;
	bool show_grid = true;
	bool snapping_enabled = true;
	bool ignore_groups = false;
	bool show_navmesh = false;
	camera_eye panned_camera;

	editor_rect_select_type rect_select_mode = editor_rect_select_type::EVERYTHING;
	// END GEN INTROSPECTOR

	void toggle_grid() {
		auto& f = show_grid;
		f = !f;
	}

	void toggle_ignore_groups() {
		auto& f = ignore_groups;
		f = !f;
	}

	void toggle_snapping() {
		auto& f = snapping_enabled;
		f = !f;
	}

	void toggle_navmesh() {
		auto& f = show_navmesh;
		f = !f;
	}

	void reset_zoom() {
		panned_camera.zoom = 1.f;
	}

	void reset_panning() {
		panned_camera = {};
	}

	void reset_zoom_at(vec2 pos) {
		panned_camera.zoom = 1.f;
		panned_camera.transform.pos = pos.discard_fract();
	}

	void center_at(vec2 pos) {
		panned_camera.transform.pos = pos.discard_fract();
	}
};
