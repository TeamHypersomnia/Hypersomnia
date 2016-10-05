#include <thread>
#include "augs/misc/templated_readwrite.h"
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/simulation_broadcast.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/cosmic_delta.h"

#include "game/transcendental/step_and_entropy_unpacker.h"

#include "augs/filesystem/file.h"

#include "setups.h"
#include "game/transcendental/network_commands.h"

#include "augs/network/reliable_channel.h"

#include "augs/misc/randomization.h"

#include "application/web_daemon/session_report.h"

#include "augs/templates.h"

#include "game/detail/position_scripts.h"

void server_setup::wait_for_listen_server() {
	std::unique_lock<std::mutex> lck(mtx);
	while (!server_ready) cv.wait(lck);
}

std::string server_setup::endpoint::nick_and_ip() const {
	return typesafe_sprintf("%x (%x)", nickname, addr.get_readable_ip());
}

server_setup::endpoint& server_setup::get_endpoint(const augs::network::endpoint_address addr) {
	return *find_in(endpoints, addr);
}

void server_setup::deinit_endpoint(endpoint& end, const bool gracefully) {
	LOG("%x disconnected.", end.nick_and_ip());

	if (hypersomnia[end.controlled_entity].alive())
		scene.free_character(end.controlled_entity);

	if(!gracefully)
		choose_server(end.addr).forceful_disconnect(end.addr);
	else
		choose_server(end.addr).disconnect(end.addr);
}

void server_setup::deinit_endpoint(const augs::network::endpoint_address addr, const bool gracefully) {
	deinit_endpoint(get_endpoint(addr), gracefully);
}

void server_setup::disconnect(const augs::network::endpoint_address addr, const bool gracefully) {
	deinit_endpoint(addr, gracefully);
	remove_element(endpoints, addr);
}

augs::network::server& server_setup::choose_server(augs::network::endpoint_address addr) {
	if (serv.has_endpoint(addr)) {
		return serv;
	}

	ensure(alternative_serv.has_endpoint(addr));
	return alternative_serv;
}

void server_setup::process(game_window& window, const bool start_alternative_server) {
	const auto& cfg = window.config;
	
	session_report rep;

	cosmos hypersomnia_last_snapshot(3000);

	cosmos initial_hypersomnia(3000);
	scene_managers::networked_testbed_server().populate_world_with_entities(initial_hypersomnia);

	step_and_entropy_unpacker input_unpacker;

	const bool detailed_step_log = cfg.tickrate <= 2;

	if (!hypersomnia.load_from_file("server_save.state")) {
		hypersomnia.set_fixed_delta(augs::fixed_delta(cfg.tickrate));
		scene.populate_world_with_entities(hypersomnia);
	}

	input_unpacker.try_to_load_or_save_new_session("server_sessions/", "server_recorded.inputs");

	simulation_broadcast server_sim;

	const bool is_replaying = input_unpacker.player.is_replaying();
	const bool launch_webserver = !is_replaying && cfg.server_launch_http_daemon;
	
	bool daemon_online = false;

	if (launch_webserver)
		daemon_online = rep.start_daemon(cfg.server_http_daemon_html_file_path, cfg.server_http_daemon_port);

	if (is_replaying || serv.listen(cfg.server_port, 32))
		LOG("Listen server setup successful.");
	else 
		LOG("Failed to setup a listen server.");

	if (start_alternative_server) {
		if (is_replaying || alternative_serv.listen(cfg.alternative_port, 32))
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
					new_endpoint.commands.set_lower_limit(static_cast<size_t>(cfg.client_commands_jitter_buffer_ms) / hypersomnia.get_fixed_delta().in_milliseconds());
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

										choose_server(address).send_reliable(complete_state, address);
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
			
			if (cfg.debug_randomize_entropies_in_client_setup) {
				for (size_t i = 1; i < scene.characters.size(); ++i) {
					if (test_entropy_randomizer.randval(0u, cfg.debug_randomize_entropies_in_client_setup_once_every_steps) == 0u) {
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

			erase_remove(endpoints, [this](endpoint& e) {
				if (choose_server(e.addr).has_timed_out(e.addr, hypersomnia.get_fixed_delta().in_milliseconds())) {
					LOG("%x has timed out. Disconnecting.", e.nick_and_ip());
					deinit_endpoint(e);
					return true;
				}

				return false;
			});

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

				choose_server(e.addr).post_redundant(new_data, e.addr);
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
					this_step_stats += "\nPlayer list:\n\n";
				}

				for (size_t i = 0; i < endpoints.size(); ++i) {
					const const_entity_handle character = hypersomnia[endpoints[i].controlled_entity];

					if (character.alive()) {
						auto pos = character.logic_transform().pos;
						auto vel = velocity(character);

						this_step_stats += typesafe_sprintf("#%x%x%x %x (%x%x%x)\nPos: %x%x%x\nVel: %x%x%x", whb, i+1, whe, endpoints[i].nickname, ipb, endpoints[i].addr.get_readable_ip(), ipe, whb, pos, whe, whb, vel, whe);
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