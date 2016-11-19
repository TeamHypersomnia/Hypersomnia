#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/networked_testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/cosmos.h"

#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmic_delta.h"
#include "game/transcendental/step.h"

#include "game/transcendental/types_specification/all_messages_includes.h"

#include "augs/filesystem/file.h"

#include "augs/network/network_client.h"

#include "augs/misc/templated_readwrite.h"
#include "client_setup.h"

void client_setup::process(game_window& window) {
	init(window);

	while (!should_quit) {
		session.local_entropy_profiler.new_measurement();
		auto precollected = window.collect_entropy();
		session.local_entropy_profiler.end_measurement();

		if (process_exit_key(precollected))
			break;

		process_once(window, precollected);
	}
}

void client_setup::init(game_window& window, const std::string recording_filename, const bool use_alternative_port) {
	const vec2i screen_size = vec2i(window.get_screen_rect());
	const auto& cfg = window.config;

	scene_managers::networked_testbed_client().populate_world_with_entities(initial_hypersomnia);

	extrapolated_hypersomnia.systems_insignificant.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;

	detailed_step_log = cfg.tickrate <= 2;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(augs::fixed_delta(cfg.tickrate));
		scene.populate_world_with_entities(hypersomnia);
	}

	if (input_unpacker.try_to_load_or_save_new_session(window.get_input_recording_mode(), "sessions/", recording_filename)) {
		input_unpacker.timer.set_stepping_speed_multiplier(cfg.recording_replay_speed);
	}

	session.camera.configure_size(screen_size);

	scene.configure_view(session);

	receiver.jitter_buffer.set_lower_limit(static_cast<unsigned>(cfg.jitter_buffer_ms / hypersomnia.get_fixed_delta().in_milliseconds()));
	receiver.misprediction_smoothing_multiplier = cfg.misprediction_smoothing_multiplier;

	const bool is_replaying = input_unpacker.player.is_replaying();
	const auto port = use_alternative_port ? cfg.alternative_port : cfg.connect_port;

	const std::string readable_ip = typesafe_sprintf("%x:%x", cfg.connect_address, port);

	LOG("Connecting to: %x", readable_ip);

	if (is_replaying || client.connect(cfg.connect_address, port, 15000)) {
		LOG("Connected successfully to %x", readable_ip);
		
		augs::stream welcome;
		augs::write_object(welcome, network_command::CLIENT_WELCOME_MESSAGE);
		augs::write_object(welcome, use_alternative_port ? cfg.debug_second_nickname : cfg.nickname);

		client.post_redundant(welcome);

		input_unpacker.timer.reset_timer();
	}
	else {
		LOG("Connection failed.");
	}
}

void client_setup::process_once(game_window& window, const augs::machine_entropy::local_type& precollected, const bool swap_buffers) {
	augs::machine_entropy new_entropy;

	new_entropy.local = precollected;
	session.remote_entropy_profiler.new_measurement();
	new_entropy.remote = client.collect_entropy();
	session.remote_entropy_profiler.end_measurement();

	const bool still_downloading = !complete_state_received || receiver.jitter_buffer.is_still_refilling();

	input_unpacker.control(new_entropy);

	session.unpack_local_steps_profiler.new_measurement();
	auto steps = input_unpacker.unpack_steps(hypersomnia.get_fixed_delta());
	session.unpack_local_steps_profiler.end_measurement();

	auto step_pred = [this](const cosmic_entropy& entropy, cosmos& cosm) {
		cosm.advance_deterministic_schemata(entropy,
			[this](logic_step&) {},
			[this](logic_step&) {}
		);
	};

	auto step_pred_with_effects_response = [this](const cosmic_entropy& entropy, cosmos& cosm) {
		cosm.advance_deterministic_schemata(entropy,
			[this](logic_step&) {},
			[this](logic_step& step) {
				session.visual_response_to_game_events(step);
			}
		);
	};

	for (auto& s : steps) {
		for (auto& net_event : s.total_entropy.remote) {
			if (net_event.message_type == augs::network::message::type::RECEIVE) {
				auto& stream = net_event.payload;

				auto to_skip = net_event.messages_to_skip;

				while (stream.get_unread_bytes() > 0) {
					network_command command;
					augs::read_object(stream, command);

					const bool should_skip = to_skip > 0;

					if (should_skip) {
						--to_skip;
					}

					if (detailed_step_log && !should_skip) {
						LOG("Client received command: %x", int(stream.peek<unsigned char>()));
					}

					switch (command) {
					case network_command::COMPLETE_STATE:
						ensure(!should_skip);

						cosmic_delta::decode(initial_hypersomnia, stream);
						hypersomnia = initial_hypersomnia;
						extrapolated_hypersomnia = initial_hypersomnia;

						LOG("Decoded cosm at step: %x", hypersomnia.get_total_steps_passed());

						unsigned controlled_character_guid;
						augs::read_object(stream, controlled_character_guid);

						scene.inject_input_to(hypersomnia.get_entity_by_guid(controlled_character_guid));

						complete_state_received = true;
						break;

					case network_command::PACKAGED_STEP: {
						step_packaged_for_network step;
						augs::read_object(stream, step);
						
						if (!should_skip) {
							receiver.acquire_next_packaged_step(step);
						}
					}
						break;

					default: LOG("Client received invalid command: %x", int(command)); stream = augs::stream(); break;

					}
				}
			}
		}

		if (!still_downloading) {
			session.sending_commands_and_predict_profiler.new_measurement();
			const auto local_cosmic_entropy_for_this_step = scene.make_cosmic_entropy(s.total_entropy.local, session.context, hypersomnia);

			receiver.send_commands_and_predict(client, local_cosmic_entropy_for_this_step, extrapolated_hypersomnia, step_pred_with_effects_response);
			session.sending_commands_and_predict_profiler.end_measurement();

			// LOG("Predicting to step: %x; predicted steps: %x", extrapolated_hypersomnia.get_total_steps_passed(), receiver.predicted_steps.size());

			session.unpack_remote_steps_profiler.new_measurement();
			receiver.unpack_deterministic_steps(scene.get_controlled_entity(), hypersomnia, hypersomnia_last_snapshot, extrapolated_hypersomnia, step_pred);
			session.unpack_remote_steps_profiler.end_measurement();
		}
		
		if (client.has_timed_out(hypersomnia.get_fixed_delta().in_milliseconds(), 2000)) {
			LOG("Connection to server timed out.");
			client.forceful_disconnect();
		}

		session.sending_packets_profiler.new_measurement();
		client.send_pending_redundant();
		session.sending_packets_profiler.end_measurement();
	}

	if (!still_downloading) {
		const auto vdt = session.frame_timer.extract_variable_delta(extrapolated_hypersomnia.get_fixed_delta(), input_unpacker.timer);
		
		extrapolated_hypersomnia.integrate_interpolated_transforms(vdt.in_seconds());
		
		session.view(extrapolated_hypersomnia, scene.get_controlled_entity(), window, vdt, client, swap_buffers);
	}
}