#include <thread>
#include "augs/misc/templated_readwrite.h"
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "game/game_window.h"

#include "game/resources/manager.h"

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

#include "augs/misc/randomization.h"

#include "setups/web/session_report.h"

#include "augs/templates.h"

#include "game/detail/position_scripts.h"

void server_setup::wait_for_listen_server() {
	std::unique_lock<std::mutex> lck(mtx);
	while (!server_ready) cv.wait(lck);
}

server_setup::endpoint& server_setup::get_endpoint(const augs::network::endpoint_address addr) {
	return *find_in(endpoints, addr);
}

void server_setup::disconnect(const augs::network::endpoint_address addr) {
	auto& end = get_endpoint(addr);

	LOG("%x (%x) disconnected.", end.nickname, addr.get_readable_ip());

	if(hypersomnia[end.controlled_entity].alive())
		scene.free_character(end.controlled_entity);

	remove_element(endpoints, addr);
}

void server_setup::process(game_window& window, const bool start_alternative_server) {
	session_report rep;

	cosmos hypersomnia_last_snapshot(3000);

	cosmos initial_hypersomnia(3000);
	scene_managers::networked_testbed_server().populate_world_with_entities(initial_hypersomnia);

	step_and_entropy_unpacker input_unpacker;

	const auto config_tickrate = static_cast<unsigned>(window.get_config_number("tickrate"));

	const bool detailed_step_log = config_tickrate <= 2;

	const bool test_randomize_entropies_in_client_setup = window.get_flag("test_randomize_entropies_in_client_setup");
	const unsigned randomize_once_every = static_cast<unsigned>(window.get_config_number("test_randomize_entropies_in_client_setup_once_every_steps"));

	if (!hypersomnia.load_from_file("server_save.state")) {
		hypersomnia.set_fixed_delta(augs::fixed_delta(config_tickrate));
		scene.populate_world_with_entities(hypersomnia);
	}

	input_unpacker.try_to_load_or_save_new_session("server_sessions/", "server_recorded.inputs");

	simulation_broadcast server_sim;

	augs::network::server serv;
	augs::network::server alternative_serv;

	const bool is_replaying = input_unpacker.player.is_replaying();
	const bool launch_webserver = !is_replaying && window.get_flag("server_launch_http_daemon");
	
	bool daemon_online = false;

	if (launch_webserver)
		daemon_online = rep.start_daemon(window.get_config_string("server_http_daemon_html_file_path"), static_cast<unsigned short>(window.get_config_number("server_http_daemon_port")));

	if (is_replaying || serv.listen(static_cast<unsigned short>(window.get_config_number("server_port")), 32))
		LOG("Listen server setup successful.");
	else 
		LOG("Failed to setup a listen server.");

	if (start_alternative_server) {
		if (is_replaying || alternative_serv.listen(static_cast<unsigned short>(window.get_config_number("alternative_port")), 32))
			LOG("Alternative listen server setup successful.");
		else
			LOG("Failed to setup an alternative listen server.");
	}

	{
		std::unique_lock<std::mutex> lck(mtx);
		server_ready = true;
		cv.notify_all();
	}

	bool resubstantiate = false;
	
	input_unpacker.timer.reset_timer();
	
	randomization test_entropy_randomizer;
	
	while (!should_quit) {
		augs::machine_entropy new_entropy;

		new_entropy.local = window.collect_entropy();
		new_entropy.remote = serv.collect_entropy();

		if (start_alternative_server) {
			augs::machine_entropy alt_entropy;
			alt_entropy.remote = alternative_serv.collect_entropy();
		
			new_entropy += alt_entropy;
		}
	
		process_exit_key(new_entropy.local);

		input_unpacker.control(new_entropy);

		auto steps = input_unpacker.unpack_steps(hypersomnia.get_fixed_delta());

		for (auto& s : steps) {
			if (detailed_step_log) {
				LOG("Server step");
			}

			for (auto& net_event : s.total_entropy.remote) {
				if (detailed_step_log) {
					LOG("Server netevent");
				}
				
				const auto& address = net_event.address;

				bool should_disconnect = false;

				if (net_event.message_type == augs::network::message::type::CONNECT) {
					LOG("%x connected to server on port %x.", address.get_readable_ip(), address.get_port());
					
					endpoint new_endpoint;
					new_endpoint.addr = net_event.address;
					new_endpoint.commands.set_lower_limit(static_cast<size_t>(window.get_config_number("client_commands_jitter_buffer_ms")) / hypersomnia.get_fixed_delta().in_milliseconds());
					endpoints.push_back(new_endpoint);
				}
				
				if (net_event.message_type == augs::network::message::type::DISCONNECT) {
					LOG("Disconnecting due to net event.");
					should_disconnect = true;
				}

				if (net_event.message_type == augs::network::message::type::RECEIVE) {
					auto& stream = net_event.payload;
					auto& endpoint = *find_in(endpoints, address);
					
					auto to_skip = net_event.messages_to_skip;

					while (stream.get_unread_bytes() > 0) {
						const bool should_skip = to_skip > 0;

						if (should_skip)
							--to_skip;

						network_command command;
						augs::read_object(stream, command);

						if (detailed_step_log && !should_skip)
							LOG("Server received command: %x", int(command));

						switch (command) {
						case network_command::CLIENT_WELCOME_MESSAGE: {
							std::string read_nickname;
							augs::read_object(stream, read_nickname);

							if (!should_skip) {
								if (endpoint.sent_welcome_message) {
									LOG("Welcome message was resent. Disconnecting.");
									should_disconnect = true;
								}
								else {
									endpoint.sent_welcome_message = true;
									endpoint.nickname = read_nickname;

									if (!should_skip) {
										LOG("%x chose nickname %x.", address.get_readable_ip(), endpoint.nickname);

										auto& complete_state = initial_hypersomnia.reserved_memory_for_serialization;
										complete_state.reset_write_pos();

										augs::write_object(complete_state, network_command::COMPLETE_STATE);

										cosmic_delta::encode(initial_hypersomnia, hypersomnia, complete_state);

										hypersomnia.complete_resubstantiation();
										resubstantiate = true;

										endpoint.controlled_entity = scene.assign_new_character();
										augs::write_object(complete_state, hypersomnia[endpoint.controlled_entity].get_guid());

										if (serv.has_endpoint(address)) {
											serv.send_reliable(complete_state, address);
										}

										if (alternative_serv.has_endpoint(address)) {
											alternative_serv.send_reliable(complete_state, address);
										}
									}
								}
							}
						}

							break;

						case network_command::CLIENT_REQUESTED_ENTROPY: {
							if (!endpoint.sent_welcome_message) {
								should_disconnect = true;
							}
							else {
								guid_mapped_entropy result;
								augs::read_object(stream, result);

								if (!should_skip)
									//endpoint.commands.push_back(result);
									endpoint.commands.acquire_new_command(result);
							}
						}
							break;
						default: 
							LOG("Server received invalid command: %x", int(command)); stream = augs::stream();
							break;
						}
					}
				}

				if (should_disconnect) {
					disconnect(address);
				}
			}

			guid_mapped_entropy total_unpacked_entropy;
			
			if (test_randomize_entropies_in_client_setup) {
				for (size_t i = 1; i < scene.characters.size(); ++i) {
					if (test_entropy_randomizer.randval(0u, randomize_once_every) == 0u) {
						const unsigned which = test_entropy_randomizer.randval(0, 4);

						entity_intent new_intent;

						switch (which) {
						case 0: new_intent.intent = intent_type::MOVE_BACKWARD; break;
						case 1: new_intent.intent = intent_type::MOVE_FORWARD; break;
						case 2: new_intent.intent = intent_type::MOVE_LEFT; break;
						case 3: new_intent.intent = intent_type::MOVE_RIGHT; break;
						//case 4: new_intent.intent = intent_type::CROSSHAIR_PRIMARY_ACTION; break;
						}

						new_intent.pressed_flag = test_entropy_randomizer.randval(0, 1) == 0;

						total_unpacked_entropy.entropy_per_entity[hypersomnia[scene.characters[i].id].get_guid()].push_back(new_intent);
					}
				}
			}

			for (auto& e : endpoints) {
				guid_mapped_entropy maybe_new_client_commands;
				auto& next_command = e.next_command;
				next_command = simulation_exchange::packaged_step();

				if (e.commands.unpack_new_command(maybe_new_client_commands)) {
					total_unpacked_entropy += maybe_new_client_commands;

					e.next_command.next_client_commands_accepted = true;
				}
			}

			for (auto& e : endpoints) {
				auto& next_command = e.next_command;

				next_command.step_type = simulation_exchange::packaged_step::type::NEW_ENTROPY;
				next_command.shall_resubstantiate = resubstantiate;
				next_command.entropy = total_unpacked_entropy;

				augs::stream new_data;
				simulation_exchange::write_packaged_step_to_stream(new_data, next_command);

				if (serv.has_endpoint(e.addr)) {
					serv.post_redundant(new_data, e.addr);
				}

				else if (alternative_serv.has_endpoint(e.addr)) {
					alternative_serv.post_redundant(new_data, e.addr);
				}
			}
			
			resubstantiate = false;

			serv.send_pending_redundant();
			if(start_alternative_server) alternative_serv.send_pending_redundant();

			cosmic_entropy id_mapped_entropy(total_unpacked_entropy, hypersomnia);
			scene.step_with_callbacks(id_mapped_entropy, hypersomnia);

			if (daemon_online) {
				std::string this_step_stats;
				const char* whb = "<span style=\"color:white\">";
				const char* whe = "</span>";

				const char* ipb = "<span class=\"vstype\">";
				const char* ipe = "</span>";

				this_step_stats += typesafe_sprintf("Uptime: %x%x%x seconds\n", whb, hypersomnia.get_total_time_passed_in_seconds(), whe);
				this_step_stats += typesafe_sprintf("Players online: %x%x%x", whb, endpoints.size(), whe);

				if (endpoints.size() > 0) {
					this_step_stats += "\nEndpoint details:\n\n";
				}

				for (size_t i = 0; i < endpoints.size(); ++i) {
					const const_entity_handle character = hypersomnia[endpoints[i].controlled_entity];

					if (character.alive()) {
						auto pos = character.logic_transform().pos;
						auto vel = velocity(character);

						this_step_stats += typesafe_sprintf("#%x%x%x %x (%x%x%x)\nPos: %x%x%x\nVel: %x%x%x", whb, i, whe, endpoints[i].nickname, ipb, endpoints[i].addr.get_readable_ip(), ipe, whb, pos, whe, whb, vel, whe);
					}

					this_step_stats += "\n\n";
				}

				this_step_stats = replace_all(this_step_stats, "\n", "\n<br/>");

				rep.fetch_stats(this_step_stats);
			}
		}
	}

	if (!is_replaying)
		rep.stop_daemon();
}