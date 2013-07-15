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

void render_system::process_entities() {
	glClear(GL_COLOR_BUFFER_BIT);

	/* we traverse layers in reverse order to keep layer 0 as topmost and last layer on the bottom */
	for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
		for(auto e = (*it).targets.begin(); e != (*it).targets.end(); ++e) {
			auto& render_info = (*e)->get<render_component>();
			auto& transform = (*e)->get<transform_component>();

			glLoadIdentity();

			glTranslatef(transform.pos.x, transform.pos.y, 0.0);
			glColor4f(render_info.r, render_info.g, render_info.b, render_info.a);

			glBegin (GL_QUADS);
			glVertex2i (0, 0);
			glVertex2i (transform.size.w, 0);
			glVertex2i (transform.size.w, transform.size.h);
			glVertex2i (0, transform.size.h);
			glEnd();
		}
	}

	output_window.swap_buffers();
}

void render_system::add(entity* e) {
	auto layer = e->get<render_component>().layer;

	if(layers.size() <= layer)
		layers.resize(layer + 1);

	layers[layer].targets.push_back(e);
}

void render_system::remove(entity* e) {
	auto layer = e->get<render_component>().layer;

	if(layers.size() > layer) {
		auto entities = layers[layer].targets; 
		entities.erase(std::remove(entities.begin(), entities.end(), e), entities.end());
	}
}

movement_system::movement_system() : accumulator(60.0, 5) {
}
void movement_system::process_entities() {
	unsigned steps = accumulator.update_and_extract_steps();

	for(unsigned i = 0; i < steps; ++i)
		for_each([this](entity* e) {
			auto& pos = e->get<transform_component>();
			auto& vel = e->get<velocity_component>();
			pos.pos += vel.vel*accumulator.per_second();
	});
}

input_system::input_system(window::glwindow& input_window, bool& quit_flag) : input_window(input_window), quit_flag(quit_flag) {
	states[0] = states[1] = states[2] = states[3] = 0;
}

void input_system::process_entities() {
	window::event::message msg;

	while(input_window.poll_events(msg)) {
		if(msg == window::event::close) {
			quit_flag = true;
		}
		if(msg == window::event::key::down || msg == window::event::key::up) {
			if(input_window.events.key == window::event::keys::ESC)
				quit_flag = true;

			bool state = (msg == window::event::key::down);
			if(input_window.events.key == window::event::keys::DOWN)	states[GO_DOWN] = state;	    
			if(input_window.events.key == window::event::keys::UP)		states[GO_UP] = state;
			if(input_window.events.key == window::event::keys::RIGHT)	states[GO_RIGHT] = state;	    
			if(input_window.events.key == window::event::keys::LEFT)	states[GO_LEFT] = state;	    
		}
	}

	for_each([this](entity* e){
		auto& vel = e->get<velocity_component>().vel;
			vel.x = ((states[GO_RIGHT]==true)*800.0f) + ((states[GO_LEFT]==true)*(-800.0f));    
			vel.y = ((states[GO_DOWN] ==true)*800.0f) + ((states[GO_UP]  ==true)*(-800.0f));		    
	});
}