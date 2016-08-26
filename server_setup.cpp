#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "game/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/simulation_broadcast.h"
#include "game/transcendental/cosmos.h"

#include "game/transcendental/step_and_entropy_unpacker.h"

#include "augs/filesystem/file.h"

#include "augs/network/network_server.h"

#include "setups.h"

void server_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.window.get_screen_rect());

	cosmos hypersomnia(3000);
	cosmos hypersomnia_last_snapshot(3000);
	cosmos extrapolated_hypersomnia(3000);

	step_and_entropy_unpacker input_unpacker;
	scene_managers::testbed testbed;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.significant.meta.settings.screen_size = screen_size;
		testbed.populate_world_with_entities(hypersomnia);
	}
	else
		hypersomnia.significant.meta.settings.screen_size = screen_size;

	input_unpacker.try_to_load_or_save_new_session("sessions/", "recorded.inputs");

	viewing_session session;
	session.camera.configure_size(screen_size);

	testbed.configure_view(session);

	simulation_broadcast server_sim;

	volatile bool should_quit = false;

	std::thread server_thread([&session, &server_sim, &should_quit]() {
		while (!should_quit) {
			//server_sim.simulate(session.input);
		}
	});

	augs::network::server serv;
	serv.listen(static_cast<unsigned short>(window.get_config_number("server_port")), 32);

	while (!should_quit) {
		augs::machine_entropy new_entropy;

		new_entropy.local = window.collect_entropy();
		new_entropy.remote = serv.collect_entropy();

		for (auto& n : new_entropy.local) {
			if (n.key == augs::window::event::keys::ESC && n.key_event == augs::window::event::key_changed::PRESSED) {
				should_quit = true;
			}
		}

		input_unpacker.control(new_entropy);

		auto steps = input_unpacker.unpack_steps(hypersomnia.get_fixed_delta());

		for (const auto& s : steps) {
			testbed.control(s.total_entropy.local, hypersomnia);

			auto cosmic_entropy_for_this_step = testbed.make_cosmic_entropy(s.total_entropy.local, session.input, hypersomnia);

			testbed.step_with_callbacks(cosmic_entropy_for_this_step, hypersomnia);

			renderer::get_current().clear_logic_lines();
		}
	}

	server_thread.join();
}