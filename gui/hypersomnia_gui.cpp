#include "stdafx.h"
#pragma once
#include "hypersomnia_gui.h"
#include "window_framework/window.h"

#include "game_framework/systems/render_system.h"


hypersomnia_gui::hypersomnia_gui(glwindow& gl) : sys(gl.events), gl(gl) {

}

void hypersomnia_gui::poll_events() {
	main_window.update_rectangles();
	main_window.poll_events();
}

void hypersomnia_gui::draw_call(resources::renderable::draw_input& in) {
	augs::graphics::fps.sumframes = delta_timer.extract<std::chrono::seconds>();
	main_window.default_update();

	in.output->triangles.insert(in.output->triangles.end(), main_window.quad_array.begin(), main_window.quad_array.end());
}