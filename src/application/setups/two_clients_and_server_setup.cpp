#include "two_clients_and_server_setup.h"
#include "server_setup.h"
#include "client_setup.h"
#include "game/transcendental/types_specification/all_component_includes.h"

#include "application/game_window.h"

void two_clients_and_server_setup::process(
	const config_lua_table& cfg, 
	game_window& window
) {
	server_setup serv_setup;

	viewing_session sessions[2];
	sessions[0].initialize(window, cfg);
	sessions[1].initialize(window, cfg);
	
	sessions[0].reserve_caches_for_entities(3000);
	sessions[1].reserve_caches_for_entities(3000);

	std::thread server_thread([&]() {
		serv_setup.process(cfg, window, true);
	});

	serv_setup.wait_for_listen_server();

	client_setup setups[2];
	setups[0].init(cfg, window, sessions[0], "recorded_0.inputs");
	setups[1].init(cfg, window, sessions[1], "recorded_1.inputs", true);

	sessions[0].camera.camera.visible_world_area.x /= 2;
	sessions[1].camera.camera.visible_world_area.x /= 2;
	
	sessions[0].viewport_coordinates = vec2i(0, 0);
	sessions[1].viewport_coordinates = vec2i(static_cast<int>(sessions[1].camera.camera.visible_world_area.x), 0);

	unsigned current_window = 0;

	bool alive[2] = { true, true };

	while (!should_quit) {
		sessions[0].local_entropy_profiler.new_measurement();
		sessions[1].local_entropy_profiler.new_measurement();
		auto precollected = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
		sessions[0].local_entropy_profiler.end_measurement();
		sessions[1].local_entropy_profiler.end_measurement();

		if (process_exit_key(precollected))
			break;

		for (const auto& n : precollected) {
			if (n.was_any_key_pressed()) {
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
		
		auto& target = augs::renderer::get_current();
		target.clear_current_fbo();

		if (current_window == 0) {
			if(alive[0]) setups[0].process_once(cfg, window, sessions[0], precollected, false);
			if(alive[1]) setups[1].process_once(cfg, window, sessions[1], augs::machine_entropy::local_type(), false);
		}

		if (current_window == 1) {
			if(alive[1]) setups[1].process_once(cfg, window, sessions[0], precollected, false);
			if(alive[0]) setups[0].process_once(cfg, window, sessions[1], augs::machine_entropy::local_type(), false);
		}

		window.swap_buffers();
	}

	serv_setup.should_quit = true;
	server_thread.join();
}