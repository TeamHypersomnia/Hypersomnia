#include "input_system.h"
#include "../../../window_framework/window.h"

input_system::input_system(window::glwindow& input_window, bool& quit_flag) : input_window(input_window), quit_flag(quit_flag) {

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
		}
	}

	for_each([this](entity* e){
		auto& vel = e->get<components::input>();
	});
}