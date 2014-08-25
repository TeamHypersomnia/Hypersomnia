#include "stdafx.h"
#pragma once
#include "hypersomnia_gui.h"
#include "window_framework/window.h"




void hypersomnia_gui::poll_events() {
	memcpy(&gl.events, &actual_window.events, sizeof(actual_window.events));

	main_window.update_rectangles();
	main_window.poll_events();
}

void hypersomnia_gui::draw_call() {
	db::graphics::fps.sumframes = delta_timer.extract<std::chrono::seconds>();
	main_window.default_update();

	atl._bind();
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad) * main_window.quad_array.size(), main_window.quad_array.data(), GL_STREAM_DRAW);
	glDrawArrays(GL_QUADS, 0, main_window.quad_array.size() * 4);
}