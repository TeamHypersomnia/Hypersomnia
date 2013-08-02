#include "input_system.h"
#include "../../../window_framework/window.h"

input_system::input_system(window::glwindow& input_window, bool& quit_flag) : input_window(input_window), quit_flag(quit_flag) {

}

void input_system::process_entities(world& owner) {
	window::event::message msg;
	unsigned incoming_event = 0;

	while (input_window.poll_events(msg)) 
	{
		if(msg == window::event::close) {
			quit_flag = true;
		}

		if(msg == window::event::key::down || msg == window::event::key::up) {
			if(input_window.events.key == window::event::keys::ESC)
				quit_flag = true;
		}

		for(auto it = targets.begin(); it != targets.end(); ++it) {
			auto& events = (*it)->get<components::input>();
			if (events.find(incoming_event)) {
				owner.post_message(incoming_event);
			}
		}
	}

}