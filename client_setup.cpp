#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "game/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/networked_testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/simulation_receiver.h"
#include "game/transcendental/cosmos.h"

#include "game/transcendental/step_and_entropy_unpacker.h"

#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmic_delta.h"

#include "augs/filesystem/file.h"

#include "augs/network/network_client.h"

#include "augs/misc/templated_readwrite.h"
#include "setups.h"

void client_setup::process(game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_rect());

	cosmos hypersomnia(3000);

	cosmos initial_hypersomnia(3000);
	scene_managers::networked_testbed_client().populate_world_with_entities(initial_hypersomnia);

	step_and_entropy_unpacker input_unpacker;
	scene_managers::networked_testbed_client scene;
	
	auto config_tickrate = static_cast<unsigned>(window.get_config_number("tickrate"));

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(augs::fixed_delta(config_tickrate));
		scene.populate_world_with_entities(hypersomnia);
	}

	input_unpacker.try_to_load_or_save_new_session("sessions/", "recorded.inputs");

	viewing_session session;
	session.camera.configure_size(screen_size);

	scene.configure_view(session);

	augs::network::client client;
	simulation_receiver receiver;

	cosmos hypersomnia_last_snapshot(3000);
	cosmos extrapolated_hypersomnia(3000);

	bool last_stepped_was_extrapolated = false;
	bool complete_state_received = false;

	if (client.connect(window.get_config_string("connect_address"), static_cast<unsigned short>(window.get_config_number("connect_port")), 15000)) {
		LOG("Connected successfully");
		
		while (!window.should_quit) {
			augs::machine_entropy new_entropy;

			new_entropy.local = window.collect_entropy();
			new_entropy.remote = client.collect_entropy();

			for (auto& n : new_entropy.local) {
				if (n.key == augs::window::event::keys::ESC && n.key_event == augs::window::event::key_changed::PRESSED) {
					window.should_quit = true;
				}
			}

			input_unpacker.control(new_entropy);

			auto steps = input_unpacker.unpack_steps(hypersomnia.get_fixed_delta());

			for (auto& s : steps) {
				for (auto& net_event : s.total_entropy.remote) {
					if (net_event.message_type == augs::network::message::type::RECEIVE) {
						auto& stream = net_event.payload;

						auto to_skip = net_event.messages_to_skip;

						while (stream.get_unread_bytes() > 0) {
							const auto command = static_cast<network_command>(stream.peek<unsigned char>());

							bool should_skip = to_skip > 0;
							
							if (should_skip)
								--to_skip;

							if(!should_skip)
								LOG("Client received command: %x", int(stream.peek<unsigned char>()));

							switch (command) {
							case network_command::COMPLETE_STATE:
								ensure(!should_skip);

								network_command read_command;

								augs::read_object(stream, read_command);
								ensure_eq(int(network_command::COMPLETE_STATE), int(read_command));

								cosmic_delta::decode(initial_hypersomnia, stream);
								hypersomnia = initial_hypersomnia;

								unsigned controlled_character_guid;
								augs::read_object(stream, controlled_character_guid);

								scene.inject_input_to(hypersomnia.get_entity_by_guid(controlled_character_guid));

								complete_state_received = true;
								break;

							case network_command::ENTROPY_FOR_NEXT_STEP:
								receiver.read_entropy_for_next_step(stream, should_skip);
								break;

							case network_command::ENTROPY_WITH_HEARTBEAT_FOR_NEXT_STEP:
								receiver.read_entropy_with_heartbeat_for_next_step(stream, should_skip);
								break;

							default: LOG("Client received invalid command: %x", int(command)); stream = augs::stream(); break;

							}
						}
					}
				}

				if (!complete_state_received)
					continue;

				const auto local_cosmic_entropy_for_this_step = scene.make_cosmic_entropy(s.total_entropy.local, session.input, hypersomnia);

				simulation_exchange::packaged_step net_step;
				net_step.step_type = simulation_exchange::packaged_step::type::NEW_ENTROPY;
				net_step.entropy = guid_mapped_entropy(local_cosmic_entropy_for_this_step, hypersomnia);

				augs::stream serialized_step;
				simulation_exchange::write_packaged_step_to_stream(serialized_step, net_step);

				client.post_redundant(serialized_step);
				client.send_pending_redundant();

				auto deterministic_steps = receiver.unpack_deterministic_steps(hypersomnia, extrapolated_hypersomnia, hypersomnia_last_snapshot);

				scene.step_with_callbacks(local_cosmic_entropy_for_this_step, hypersomnia);

				if (deterministic_steps.use_extrapolated_cosmos) {
					scene.step_with_callbacks(cosmic_entropy(), extrapolated_hypersomnia);
					renderer::get_current().clear_logic_lines();
					last_stepped_was_extrapolated = true;
				}
				else {
					last_stepped_was_extrapolated = false;
				
					ensure(deterministic_steps.has_next_entropy());
				
					while (deterministic_steps.has_next_entropy()) {
						const auto cosmic_entropy_for_this_step = deterministic_steps.unpack_next_entropy(hypersomnia);
						scene.step_with_callbacks(cosmic_entropy_for_this_step, hypersomnia);
						renderer::get_current().clear_logic_lines();
					}
				}
			}

			scene.view(last_stepped_was_extrapolated ? extrapolated_hypersomnia : hypersomnia,
				window, session, session.frame_timer.extract_variable_delta(hypersomnia.get_fixed_delta(), input_unpacker.timer));
		}
	}
	else {
		LOG("Connection failed.");
	}
}