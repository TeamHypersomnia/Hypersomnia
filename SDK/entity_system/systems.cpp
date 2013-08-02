#include <gl/glew.h>
#include <functional>

#include "systems.h"

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

void render_system::process_entities(world&) {
	glClear(GL_COLOR_BUFFER_BIT);

	std::vector<entity*> visible_targets = targets;

	std::sort(visible_targets.begin(), visible_targets.end(), [](entity* a, entity* b){ 
		return a->get<render_component>().layer < b->get<render_component>().layer;
	});


	/* we traverse layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	for (auto e = visible_targets.begin(); e != visible_targets.end(); ++e) {
		auto& render_info = (*e)->get<render_component>();
		auto& transform = (*e)->get<transform_component>();

		glLoadIdentity();

		glTranslatef(transform.pos.x, transform.pos.y, 0.0);
		glColor4f(render_info.r, render_info.g, render_info.b, render_info.a);

		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(transform.size.w, 0);
		glVertex2i(transform.size.w, transform.size.h);
		glVertex2i(0, transform.size.h);
		glEnd();
	}

	output_window.swap_buffers();
}

/* because we're consuming velocity only ONCE 1000/fps milliseconds, input seems to be discrete in low framerates */
movement_system::movement_system() : accumulator(100.0, 5) {
}
void movement_system::process_entities(world&) {
	unsigned steps = accumulator.update_and_extract_steps();

	for(unsigned i = 0; i < steps; ++i)
		for (auto it = targets.begin(); it != targets.end(); ++it) {
			auto& pos = (*it)->get<transform_component>();
			auto& vel = (*it)->get<velocity_component>();
			pos.pos += vel.vel*accumulator.per_second();
	}
}

input_system::input_system(window::glwindow& input_window, bool& quit_flag) : input_window(input_window), quit_flag(quit_flag) {
	states[0] = states[1] = states[2] = states[3] = 0;
}

void input_system::process_entities(world&) {
	window::event::message msg;

	while(input_window.poll_events(msg)) {
		if(msg == window::event::close) {
			quit_flag = true;
		}
		if(msg == window::event::key::down || msg == window::event::key::up) {
			if(input_window.events.key == window::event::keys::ESC)
				quit_flag = true;

			bool state = (msg == window::event::key::down);
			if(input_window.events.key == window::event::keys::S)	{ states[GO_DOWN] = state;	}    
			if(input_window.events.key == window::event::keys::W)	{ states[GO_UP] = state;	}
			if(input_window.events.key == window::event::keys::D)	{ states[GO_RIGHT] = state;	}
			if(input_window.events.key == window::event::keys::A)	{ states[GO_LEFT] = state;	}
		}
	}

	for(auto it = targets.begin(); it != targets.end(); ++it) {
		auto& vel = (*it)->get<velocity_component>().vel;
			vel.x = ((states[GO_RIGHT]==true)*800.0f) + ((states[GO_LEFT]==true)*(-800.0f));    
			vel.y = ((states[GO_DOWN] ==true)*800.0f) + ((states[GO_UP]  ==true)*(-800.0f));		    
	}
}