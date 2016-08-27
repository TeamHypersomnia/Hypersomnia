#include <thread>
#include "augs/misc/templated_readwrite.h"
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
#include "game/transcendental/cosmic_delta.h"

#include "game/transcendental/step_and_entropy_unpacker.h"

#include "augs/filesystem/file.h"

#include "augs/network/network_server.h"

#include "setups.h"
#include "game/transcendental/network_commands.h"

#include "augs/network/reliable_channel.h"

void server_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.window.get_screen_rect());

	cosmos hypersomnia(3000);
	cosmos hypersomnia_last_snapshot(3000);
	cosmos extrapolated_hypersomnia(3000);

	cosmos initial_hypersomnia(3000);
	scene_managers::testbed().populate_world_with_entities(initial_hypersomnia);

	step_and_entropy_unpacker input_unpacker;
	scene_managers::testbed testbed;

	if (!hypersomnia.load_from_file("save.state")) {
		testbed.populate_world_with_entities(hypersomnia);
	}

	input_unpacker.try_to_load_or_save_new_session("sessions/", "recorded.inputs");

	viewing_session session;
	session.camera.configure_size(screen_size);

	testbed.configure_view(session);

	simulation_broadcast server_sim;

	volatile bool should_quit = false;

	augs::network::server serv;
	serv.listen(static_cast<unsigned short>(window.get_config_number("server_port")), 32);

	struct endpoint {
		augs::network::endpoint_address addr;
		std::vector<simulation_exchange::packaged_step> commands;

		bool operator==(augs::network::endpoint_address b) {
			return addr == b;
		}
	};

	std::vector<endpoint> endpoints;

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

		for (auto& s : steps) {
			for (auto& net_event : s.total_entropy.remote) {
				if (net_event.message_type == augs::network::message::type::CONNECT) {
					endpoints.push_back({ net_event.address });

					auto& stream = initial_hypersomnia.reserved_memory_for_serialization;
					stream.reset_write_pos();

					augs::write_object(stream, network_command::COMPLETE_STATE);
					cosmic_delta::encode(initial_hypersomnia, hypersomnia, stream);

					serv.send_reliable(stream, net_event.address);
				}
				
				if (net_event.message_type == augs::network::message::type::DISCONNECT) {
					remove_element(endpoints, net_event.address);
				}

				if (net_event.message_type == augs::network::message::type::RECEIVE) {
					auto& stream = net_event.payload;
					auto& endpoint = *find_in(endpoints, net_event.address);

					while (stream.get_unread_bytes() > 0) {
						auto command = static_cast<network_command>(stream.peek<unsigned char>());

						switch (command) {
						case network_command::ENTROPY_FOR_NEXT_STEP:
							endpoint.commands.push_back(simulation_exchange::read_entropy_for_next_step(stream));
							break;
						}
					}

					ensure(net_event.payload.get_unread_bytes() == 0);
				}
			}

			simulation_exchange::packaged_step this_net_step;
			this_net_step.step_type = simulation_exchange::packaged_step::type::NEW_ENTROPY;
			auto& total_entropy = this_net_step.entropy;
			
			for (auto& e : endpoints) {
				auto next_command = e.commands.front();
				e.commands.erase(e.commands.begin());

				ensure(next_command.step_type == simulation_exchange::packaged_step::type::NEW_ENTROPY);
				
				total_entropy += next_command.entropy;
			}

			augs::stream new_data;
			simulation_exchange::write_packaged_step_to_stream(new_data, this_net_step, hypersomnia);

			for (auto& e : endpoints)
				serv.post_redundant(new_data, e.addr);

			serv.send_pending_redundant();

			testbed.step_with_callbacks(total_entropy, hypersomnia);
		}
	}
}