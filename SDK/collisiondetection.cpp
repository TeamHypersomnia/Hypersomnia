#pragma once
#include "rb.h"

using namespace db;
using namespace window;

int main() {
	glwindow::init();
	glwindow gl;

	config window_config("window_config.txt");

	gl.create(window_config, math::rect_xywh(40, 40, 600, 600), glwindow::ALL); 
	gl.set_show(gl.SHOW);
	
	event::message msg;
	graphics::init();

	gui::system sys(&gl.events);

	bool quit = false;
	while(!quit) {
		while(gl.poll_events(msg)) {
			if(msg == event::close) {
				quit = true;
			}
		}
		
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glBegin(GL_TRIANGLES);
		glVertex2f(1.0, 0.0);
		glVertex2f(1.0, 1.0);
		glVertex2f(0.0, 1.0);
		glEnd();

		gl.swap_buffers();
	}
	
	return 0;
}