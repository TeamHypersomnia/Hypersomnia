#include <thread>
#include "augs/misc/templated_readwrite.h"
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "game/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/networked_testbed.h"
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

void server_setup::wait_for_listen_server() {
	std::unique_lock<std::mutex> lck(mtx);
	while (!server_ready) cv.wait(lck);
}

void server_setup::process(game_window& window) {
	cosmos hypersomnia(3000);
	cosmos hypersomnia_last_snapshot(3000);
	cosmos extrapolated_hypersomnia(3000);

	cosmos initial_hypersomnia(3000);
	scene_managers::networked_testbed_server().populate_world_with_entities(initial_hypersomnia);

	step_and_entropy_unpacker input_unpacker;
	scene_managers::networked_testbed_server scene;

	auto config_tickrate = static_cast<unsigned>(window.get_config_number("tickrate"));

	bool detailed_step_log = config_tickrate <= 2;

	if (!hypersomnia.load_from_file("server_save.state")) {
		hypersomnia.set_fixed_delta(augs::fixed_delta(config_tickrate));
		scene.populate_world_with_entities(hypersomnia);
	}

	input_unpacker.try_to_load_or_save_new_session("server_sessions/", "server_recorded.inputs");

	simulation_broadcast server_sim;

	augs::network::server serv;

	const bool is_replaying = input_unpacker.player.is_replaying();

	if (is_replaying || serv.listen(static_cast<unsigned short>(window.get_config_number("server_port")), 32))
		LOG("Listen server setup successful.");
	else 
		LOG("Failed to setup a listen server.");

	{
		std::unique_lock<std::mutex> lck(mtx);
		server_ready = true;
		cv.notify_all();
	}

	struct endpoint {
		augs::network::endpoint_address addr;
		std::vector<guid_mapped_entropy> commands;

		bool operator==(augs::network::endpoint_address b) const {
			return addr == b;
		}
	};

	std::vector<endpoint> endpoints;

	bool resubstantiate = false;
	
	input_unpacker.timer.reset_timer();

	while (!should_quit) {
		augs::machine_entropy new_entropy;

		new_entropy.local = window.collect_entropy();
		new_entropy.remote = serv.collect_entropy();
	
		process_exit_key(new_entropy.local);

		input_unpacker.control(new_entropy);

		auto steps = input_unpacker.unpack_steps(hypersomnia.get_fixed_delta());

		for (auto& s : steps) {
			if (detailed_step_log) LOG("Server step");
			for (auto& net_event : s.total_entropy.remote) {
				if(detailed_step_log) LOG("Server netevent");

				if (net_event.message_type == augs::network::message::type::CONNECT) {
					LOG("Client connected.");
					
					endpoints.push_back({ net_event.address });

					auto& stream = initial_hypersomnia.reserved_memory_for_serialization;
					stream.reset_write_pos();

					augs::write_object(stream, network_command::COMPLETE_STATE);
					
					cosmic_delta::encode(initial_hypersomnia, hypersomnia, stream);

					hypersomnia.complete_resubstantiation();
					resubstantiate = true;

					auto new_char = scene.assign_new_character(net_event.address);
					augs::write_object(stream, hypersomnia[new_char].get_guid());

					serv.send_reliable(stream, net_event.address);
				}
				
				if (net_event.message_type == augs::network::message::type::DISCONNECT) {
					LOG("Client disconnected.");
					scene.free_character(net_event.address);

					remove_element(endpoints, net_event.address);
				}

				if (net_event.message_type == augs::network::message::type::RECEIVE) {
					auto& stream = net_event.payload;
					auto& endpoint = *find_in(endpoints, net_event.address);
					
					auto to_skip = net_event.messages_to_skip;

					while (stream.get_unread_bytes() > 0) {
						bool should_skip = to_skip > 0;

						if (should_skip)
							--to_skip;

						auto command = static_cast<network_command>(stream.peek<unsigned char>());

						if (detailed_step_log && !should_skip)
							LOG("Server received command: %x", int(command));

						switch (command) {
						case network_command::CLIENT_REQUESTED_ENTROPY:
						{
							network_command com;
							augs::read_object(stream, com);

							ensure_eq(int(network_command::CLIENT_REQUESTED_ENTROPY), int(com));

							guid_mapped_entropy result;
							augs::read_object(stream, result);
								
							if(!should_skip)
								endpoint.commands.push_back(result);
						}
							break;
						default: 
							LOG("Server received invalid command: %x", int(command)); stream = augs::stream();
							break;
						}
					}
				}
			}

			simulation_exchange::packaged_step this_net_step;
			this_net_step.step_type = simulation_exchange::packaged_step::type::NEW_ENTROPY;
			auto& total_entropy = this_net_step.entropy;
			
			for (auto& e : endpoints) {
				simulation_exchange::packaged_step next_command;
				next_command.step_type = simulation_exchange::packaged_step::type::NEW_ENTROPY;
				next_command.shall_resubstantiate = resubstantiate;
				resubstantiate = false;

				if (e.commands.size() > 0) {
					next_command.entropy = e.commands.front();
					e.commands.erase(e.commands.begin());
				}

				ensure(next_command.step_type == simulation_exchange::packaged_step::type::NEW_ENTROPY);
				
				total_entropy += next_command.entropy;
			}

			augs::stream new_data;
			simulation_exchange::write_packaged_step_to_stream(new_data, this_net_step);

			for (auto& e : endpoints)
				serv.post_redundant(new_data, e.addr);

			serv.send_pending_redundant();

			cosmic_entropy id_mapped_entropy(total_entropy, hypersomnia);
			scene.step_with_callbacks(id_mapped_entropy, hypersomnia);
		}
	}
}