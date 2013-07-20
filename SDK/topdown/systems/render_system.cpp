#pragma once
#include "render_system.h"
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

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

void render_system::process_entities() {
	quads.clear();
	using namespace rects;
	/* we traverse layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
		for(auto e = (*it).targets.begin(); e != (*it).targets.end(); ++e) {
			auto& render_info = (*e)->get<components::render>();
			auto& transform = (*e)->get<components::transform>();

			quad q;
			q.vertices[0].position = (transform.pos + (pointf(transform.size)*pointf(-0.5, -0.5))).rotate(transform.rotation, transform.pos);
			q.vertices[1].position = (transform.pos + (pointf(transform.size)*pointf( 0.5, -0.5))).rotate(transform.rotation, transform.pos);
			q.vertices[2].position = (transform.pos + (pointf(transform.size)*pointf( 0.5,  0.5))).rotate(transform.rotation, transform.pos);
			q.vertices[3].position = (transform.pos + (pointf(transform.size)*pointf(-0.5,  0.5))).rotate(transform.rotation, transform.pos);

			quads.emplace_back(q);
		}
	}

	glClear(GL_COLOR_BUFFER_BIT);
	
	glVertexPointer(2, GL_INT, sizeof(quad::vertex), quads.data());
	//glTexCoordPointer(2, GL_FLOAT, sizeof(quad::vertex), (char*)(quads.data()) + sizeof(int)*2);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(quad::vertex), (char*)(quads.data()) + sizeof(int)*2 + sizeof(float)*2);
	
	glDrawArrays(GL_QUADS, 0, quads.size() * 4);

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