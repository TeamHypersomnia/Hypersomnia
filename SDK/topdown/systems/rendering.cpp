#pragma once
#include "rendering.h"
#include <gl\glew.h>

render_system::render_system(window::glwindow& output_window) : output_window(output_window) {
	output_window.current();

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glViewport(0, 0, output_window.get_window_rect().w, output_window.get_window_rect().h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, output_window.get_window_rect().w, output_window.get_window_rect().h, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
}

void render_system::process_entities() {
	quads.clear();

	/* we traverse layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
		for(auto e = (*it).targets.begin(); e != (*it).targets.end(); ++e) {
			auto& render_info = (*e)->get<components::render>();
			auto& transform = (*e)->get<components::transform>();


		}
	}

	glClear(GL_COLOR_BUFFER_BIT);

	output_window.swap_buffers();
}

void render_system::add(entity* e) {
	auto layer = e->get<components::render>().layer;

	if(layers.size() <= layer)
		layers.resize(layer + 1);

	layers[layer].targets.push_back(e);
}

void render_system::remove(entity* e) {
	auto layer = e->get<components::render>().layer;

	if(layers.size() > layer) {
		auto entities = layers[layer].targets; 
		entities.erase(std::remove(entities.begin(), entities.end(), e), entities.end());
	}
}