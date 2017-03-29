#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_builders/networked_testbed.h"
#include "game/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/view/viewing_session.h"
#include "game/transcendental/cosmos.h"

#include "game/transcendental/network_commands.h"
#include "game/transcendental/cosmic_delta.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/transcendental/types_specification/all_messages_includes.h"

#include "augs/filesystem/file.h"

#include "augs/network/network_client.h"

#include "augs/misc/templated_readwrite.h"
#include "client_setup.h"
#include "game/detail/visible_entities.h"

#include "generated_introspectors.h"
#include "application/config_lua_table.h"

void client_setup::process(
	const config_lua_table& cfg, 
	game_window& window,
	viewing_session& session
) {
	init(cfg, window, session);

	while (!should_quit) {
		session.local_entropy_profiler.new_measurement();
		auto precollected = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
		session.local_entropy_profiler.end_measurement();

		if (process_exit_key(precollected))
			break;

		process_once(cfg, window, session, precollected);
	}
}

void client_setup::init(
	const config_lua_table& cfg, 
	game_window& window,
	viewing_session& session,
	const std::string recording_filename, 
	const bool use_alternative_port
) {
	scene_builders::networked_testbed_client().populate_world_with_entities(initial_hypersomnia);

	session.reserve_caches_for_entities(3000);

	detailed_step_log = cfg.default_tickrate <= 2;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(cfg.default_tickrate);
		scene.populate_world_with_entities(hypersomnia);
	}

	if (cfg.get_input_recording_mode() != input_recording_type::DISABLED) {
		//if (player.try_to_load_or_save_new_session("generated/sessions/", recording_filename)) {
		//	timer.set_stepping_speed_multiplier(cfg.recording_replay_speed);
		//}
	}

	receiver.jitter_buffer.set_lower_limit(static_cast<unsigned>(cfg.jitter_buffer_ms / hypersomnia.get_fixed_delta().in_milliseconds()));
	receiver.misprediction_smoothing_multiplier = cfg.misprediction_smoothing_multiplier;

	const bool is_replaying =
		false;// player.is_replaying();
	LOG("Is client replaying: %x", is_replaying);
	const auto port = use_alternative_port ? cfg.alternative_port : cfg.connect_port;

	const std::string readable_ip = typesafe_sprintf("%x:%x", cfg.connect_address, port);

	LOG("Connecting to: %x", readable_ip);

	if (is_replaying || client.connect(cfg.connect_address, port, 15000)) {
		LOG("Connected successfully to %x", readable_ip);
		
		augs::stream welcome;
		augs::write(welcome, network_command::CLIENT_WELCOME_MESSAGE);
		augs::write(welcome, use_alternative_port ? cfg.debug_second_nickname : cfg.nickname);

		client.post_redundant(welcome);

		timer.reset_timer();
	}
	else {
		LOG("Connection failed.");
	}
}

void client_setup::process_once(
	const config_lua_table& cfg,
	game_window& window,
	viewing_session& session,
	const augs::machine_entropy::local_type& precollected, 
	const bool swap_buffers
) {
	auto step_pred = [this](
		const cosmic_entropy& entropy, 
		cosmos& cosm
	) {
		cosm.advance_deterministic_schemata(
			entropy,
			[this](const logic_step) {},
			[this](const const_logic_step step) {}
		);
	};

	auto step_pred_with_audiovisual_response = [this, &session](
		const cosmic_entropy& entropy, 
		cosmos& cosm
	) {
		cosm.advance_deterministic_schemata(
			entropy,
			[this](const logic_step) {},
			[this, &session](const const_logic_step step) {
				session.spread_past_infection(step);
				session.standard_audiovisual_post_solve(step);
			}
		);
	};

	augs::machine_entropy new_machine_entropy;

	new_machine_entropy.local = precollected;
	session.remote_entropy_profiler.new_measurement();
	new_machine_entropy.remote = client.collect_entropy();
	session.remote_entropy_profiler.end_measurement();

	const bool still_downloading = !complete_state_received || receiver.jitter_buffer.is_still_refilling();
	
	session.switch_between_gui_and_back(new_machine_entropy.local);

	session.control_gui_and_remove_fetched_events(
		hypersomnia[scene.get_selected_character()],
		new_machine_entropy.local
	);

	auto new_intents = session.context.to_key_and_mouse_intents(new_machine_entropy.local);

	session.control_and_remove_fetched_intents(new_intents);

	auto new_cosmic_entropy = cosmic_entropy(
		hypersomnia[scene.get_selected_character()],
		new_intents
	);

	new_cosmic_entropy += session.systems_audiovisual.get<gui_element_system>().get_and_clear_pending_events();

	total_collected_entropy += new_cosmic_entropy;

	for (auto& net_event : new_machine_entropy.remote) {
		if (net_event.message_type == augs::network::message::type::RECEIVE) {
			auto& stream = net_event.payload;

			auto to_skip = net_event.messages_to_skip;

			while (stream.get_unread_bytes() > 0) {
				network_command command;
				augs::read(stream, command);

				const bool should_skip = to_skip > 0;

				if (should_skip) {
					--to_skip;
				}

				if (detailed_step_log && !should_skip) {
					LOG("Client received command: %x", static_cast<int>(stream.peek<unsigned char>()));
				}

				switch (command) {
				case network_command::COMPLETE_STATE:
					ensure(!should_skip);

					cosmic_delta::decode(initial_hypersomnia, stream);
					hypersomnia = initial_hypersomnia;
					extrapolated_hypersomnia = initial_hypersomnia;

					LOG("Decoded cosm at step: %x", hypersomnia.get_total_steps_passed());

					unsigned controlled_character_guid;
					augs::read(stream, controlled_character_guid);

					scene.select_character(hypersomnia.get_handle(controlled_character_guid));

					complete_state_received = true;
					break;

				case network_command::PACKAGED_STEP: {
					step_packaged_for_network step;
					augs::read(stream, step);

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

	auto steps = timer.count_logic_steps_to_perform(hypersomnia.get_fixed_delta());

	while (steps--) {
		if (!still_downloading) {
			session.sending_commands_and_predict_profiler.new_measurement();
			
			receiver.send_commands_and_predict(
				client, 
				total_collected_entropy,
				extrapolated_hypersomnia, 
				step_pred_with_audiovisual_response
			);

			session.sending_commands_and_predict_profiler.end_measurement();

			// LOG("Predicting to step: %x; predicted steps: %x", extrapolated_hypersomnia.get_total_steps_passed(), receiver.predicted_steps.size());

			session.unpack_remote_steps_profiler.new_measurement();
			
			receiver.unpack_deterministic_steps(
				session.systems_audiovisual.get<interpolation_system>(),
				session.systems_audiovisual.get<past_infection_system>(),
				scene.get_selected_character(), 
				hypersomnia, 
				hypersomnia_last_snapshot, 
				extrapolated_hypersomnia, 
				step_pred
			);

			session.unpack_remote_steps_profiler.end_measurement();
		}
		
		if (client.has_timed_out(hypersomnia.get_fixed_delta().in_milliseconds(), 2000)) {
			LOG("Connection to server timed out.");
			client.forceful_disconnect();
		}

		session.sending_packets_profiler.new_measurement();
		client.send_pending_redundant();
		session.sending_packets_profiler.end_measurement();

		total_collected_entropy.clear();
	}

	if (!still_downloading) {
		const auto vdt = session.frame_timer.extract_variable_delta(extrapolated_hypersomnia.get_fixed_delta(), timer);
		
		thread_local visible_entities all_visible;
		session.get_visible_entities(all_visible, hypersomnia);

		session.advance_audiovisual_systems(
			extrapolated_hypersomnia, 
			scene.get_selected_character(),
			all_visible,
			vdt
		);

		auto& renderer = augs::renderer::get_current();
		
		if (swap_buffers) {
			renderer.clear_current_fbo();
		}

		session.view(
			cfg,
			renderer,
			extrapolated_hypersomnia,
			scene.get_selected_character(),
			all_visible,
			timer.fraction_of_step_until_next_step(extrapolated_hypersomnia.get_fixed_delta()),
			client
		);

		if (swap_buffers) {
			window.swap_buffers();
		}
	}
}