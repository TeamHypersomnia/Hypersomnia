#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "game/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/simulation_exchange.h"
#include "game/transcendental/cosmos.h"

#include "game/transcendental/step_and_entropy_unpacker.h"

#include "game/transcendental/network_commands.h"

#include "augs/filesystem/file.h"

#include "augs/network/network_client.h"

#include "setups.h"

void client_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.window.get_screen_rect());

	cosmos hypersomnia(3000);

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

	augs::network::client client;
	simulation_receiver receiver;
	cosmos hypersomnia_last_snapshot(3000);
	cosmos extrapolated_hypersomnia(3000);

	bool should_quit = false;

	if (client.connect(window.get_config_string("server_address"), static_cast<unsigned short>(window.get_config_number("server_port")), 2000)) {
		LOG("Connected successfully");
		
		while (!should_quit) {
			augs::machine_entropy new_entropy;

			new_entropy.local = window.collect_entropy();
			new_entropy.remote = client.collect_entropy();

			for (auto& n : new_entropy.local) {
				if (n.key == augs::window::event::keys::ESC && n.key_event == augs::window::event::key_changed::PRESSED) {
					should_quit = true;
				}
			}

			input_unpacker.control(new_entropy);

			auto steps = input_unpacker.unpack_steps(hypersomnia.get_fixed_delta());

			for (auto& s : steps) {
				for (auto& net_event : s.total_entropy.remote) {
					if (net_event.message_type == augs::network::message::type::RECEIVE) {
						auto& stream = net_event.payload;
						
						while (stream.get_unread_bytes() > 0) {
							auto command = static_cast<network_command>(stream.peek<unsigned char>());
							
							switch (command) {
							case network_command::ENTROPY_FOR_NEXT_STEP:
								receiver.read_entropy_for_next_step(stream);
								break;

							case network_command::ENTROPY_WITH_HEARTBEAT_FOR_NEXT_STEP:
								receiver.read_entropy_with_heartbeat_for_next_step(stream);
								break;
							}
						}

						ensure(net_event.payload.get_unread_bytes() == 0);
					}

				}

				testbed.control(s.total_entropy, hypersomnia);

				auto local_cosmic_entropy_for_this_step = testbed.make_cosmic_entropy(s.total_entropy, session.input, hypersomnia);

				auto deterministic_steps = receiver.unpack_deterministic_steps(hypersomnia, extrapolated_hypersomnia, hypersomnia_last_snapshot);

				while (deterministic_steps.has_next_entropy()) {
					auto cosmic_entropy_for_this_step = deterministic_steps.unpack_next_entropy(hypersomnia);
					testbed.step_with_callbacks(cosmic_entropy_for_this_step, hypersomnia);
					renderer::get_current().clear_logic_lines();
				}
			}

			testbed.view(hypersomnia, window, session, session.frame_timer.extract_variable_delta(hypersomnia.get_fixed_delta(), input_unpacker.timer));
		}
	}
	else {
		LOG("Connection failed.");
	}
}