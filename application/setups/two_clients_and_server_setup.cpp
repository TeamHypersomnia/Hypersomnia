#include "two_clients_and_server_setup.h"
#include "server_setup.h"
#include "client_setup.h"
#include "game/transcendental/types_specification/all_component_includes.h"

void two_clients_and_server_setup::process(game_window& window) {
	server_setup serv_setup;

	std::thread server_thread([&window, &serv_setup]() {
		serv_setup.process(window, true);
	});

	serv_setup.wait_for_listen_server();

	client_setup setups[2];
	setups[0].init(window, "recorded_0.inputs");
	setups[1].init(window, "recorded_1.inputs", true);

	setups[0].session.camera.visible_world_area.x /= 2;
	setups[1].session.camera.visible_world_area.x /= 2;
	
	setups[0].session.viewport_coordinates = vec2i(0, 0);
	setups[1].session.viewport_coordinates = vec2i(static_cast<int>(setups[1].session.camera.visible_world_area.x), 0);

	unsigned current_window = 0;

	bool alive[2] = { true, true };

	while (!should_quit) {
		auto precollected = window.collect_entropy();

		if (process_exit_key(precollected))
			break;

		for (const auto& n : precollected) {
			if (n.was_key_pressed()) {
				if (n.key == augs::window::event::keys::key::CAPSLOCK) {
					++current_window;
					current_window %= 2;

					if (!alive[current_window]) {
						++current_window;
						current_window %= 2;
					}
				}
				
				if (n.key == augs::window::event::keys::key::F1) {
					alive[0] = false;
					current_window = 1;
				}

				if (n.key == augs::window::event::keys::key::F2) {
					alive[1] = false;
					current_window = 0;
				}
			}
		}

		if (!alive[0] && !alive[1]) {
			should_quit = true;
			break;
		}
		
		auto& target = renderer::get_current();
		target.clear_current_fbo();

		if (current_window == 0) {
			if(alive[0]) setups[0].process_once(window, precollected, false);
			if(alive[1]) setups[1].process_once(window, augs::machine_entropy::local_type(), false);
		}

		if (current_window == 1) {
			if(alive[1]) setups[1].process_once(window, precollected, false);
			if(alive[0]) setups[0].process_once(window, augs::machine_entropy::local_type(), false);
		}

		window.swap_buffers();
	}

	serv_setup.should_quit = true;
	server_thread.join();
}