#include <thread>
#include "game/transcendental/cosmos.h"
#include "augs/global_libraries.h"
#include "augs/window_framework/window.h"

#include "game/assets/assets_manager.h"

#include "game/organization/all_component_includes.h"
#include "view/viewing_session.h"

#include "view/network/network_commands.h"
#include "game/transcendental/cosmic_delta.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/organization/all_messages_includes.h"

#include "augs/filesystem/file.h"

#include "augs/network/network_client.h"

#include "augs/readwrite/byte_readwrite.h"
#include "client_setup.h"
#include "game/detail/visible_entities.h"


#include "application/config_lua_table.h"


void client_setup::process(
	game_window& window,
	viewing_session& session
) {
	init(window, session);

	while (!should_quit) {
		get_profiler().local_entropy.start();
		auto precollected = window.collect_entropy();
		get_profiler().local_entropy.stop();

		if (process_exit(precollected))
			break;

		process_once(window, session, precollected);
	}
}

void client_setup::init(
	game_window& window,
	viewing_session& session,
	const std::string recording_filename, 
	const bool use_alternative_port
) {
	metas_of_assets = *get_assets_manager().generate_logical_metas_of_assets();

	detailed_step_log = session.config.default_tickrate <= 2;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(session.config.default_tickrate);
	}

	if (session.config.get_input_recording_mode() != input_recording_type::DISABLED) {
		//if (player.try_to_load_or_save_new_session(LOCAL_FILES_DIR "sessions/", recording_filename)) {
		//	timer.set_stepping_speed_multiplier(1.f);
		//}
	}

	receiver.jitter_buffer.set_lower_limit(static_cast<unsigned>(session.config.jitter_buffer_ms / hypersomnia.get_fixed_delta().in_milliseconds()));
	receiver.misprediction_smoothing_multiplier = static_cast<float>(session.config.misprediction_smoothing_multiplier);

	const bool is_replaying =
		false;// player.is_replaying();
	LOG("Is client replaying: %x", is_replaying);
	const auto port = use_alternative_port ? session.config.alternative_port : session.config.connect_port;

	const std::string readable_ip = typesafe_sprintf("%x:%x", session.config.connect_address, port);

	LOG("Connecting to: %x", readable_ip);

	if (is_replaying || client.connect(session.config.connect_address, port, 15000)) {
		LOG("Connected successfully to %x", readable_ip);
		
		augs::stream welcome;
		augs::write(welcome, network_command::CLIENT_WELCOME_MESSAGE);
		augs::write(welcome, use_alternative_port ? "mysecondnickname" : "mynickname");

		client.post_redundant(welcome);

		timer.reset_timer();
	}
	else {
		LOG("Connection failed.");
	}
}

void client_setup::process_once(
	game_window& window,
	viewing_session& session,
	const augs::machine_entropy::local_type& precollected, 
	const bool swap_buffers
) {
	auto step_callback = [this](
		const cosmic_entropy& entropy, 
		cosmos& cosm
	) {
		cosm.advance_deterministic_schemata(
			{ entropy, metas_of_assets },
			[this](const logic_step) {},
			[this](const const_logic_step step) {}
		);
	};

	auto step_callback_with_audiovisual_response = [this, &session](
		const cosmic_entropy& entropy, 
		cosmos& cosm
	) {
		cosm.advance_deterministic_schemata(
			{ entropy, metas_of_assets },
			[this](const logic_step) {},
			[this, &session](const const_logic_step step) {
				session.spread_past_infection(step);
				session.standard_audiovisual_post_solve(step);
			}
		);
	};

	augs::machine_entropy new_machine_entropy;

	new_machine_entropy.local = precollected;
	
	{
		auto scope = measure_scope(get_profiler().remote_entropy);
		new_machine_entropy.remote = client.collect_entropy();
	}

	const bool still_downloading = !complete_state_received || receiver.jitter_buffer.is_still_refilling();
	
	session.switch_between_gui_and_back(new_machine_entropy.local);

	session.fetch_gui_events(
		hypersomnia[currently_controlled_character],
		new_machine_entropy.local
	);

	auto translated = session.config.controls.translate(new_machine_entropy.local);

	session.fetch_session_intents(translated.intents);

	auto new_cosmic_entropy = cosmic_entropy(
		hypersomnia[currently_controlled_character],
		translated
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

					currently_controlled_character = hypersomnia.get_handle(controlled_character_guid);

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
			{
				auto scope = measure_scope(get_profiler().sending_commands_and_predict);
				
				receiver.send_commands_and_predict(
					client,
					total_collected_entropy,
					extrapolated_hypersomnia,
					step_callback_with_audiovisual_response
				);
			}

			// LOG("Predicting to step: %x; predicted steps: %x", extrapolated_hypersomnia.get_total_steps_passed(), receiver.predicted_step_entropies.size());

			{
				auto scope = measure_scope(get_profiler().unpack_remote_steps);

				receiver.unpack_deterministic_steps(
					session.systems_audiovisual.get<interpolation_system>(),
					session.systems_audiovisual.get<past_infection_system>(),
					currently_controlled_character,
					hypersomnia,
					hypersomnia_last_snapshot,
					extrapolated_hypersomnia,
					step_callback
				);
			}
		}
		
		if (client.has_timed_out(hypersomnia.get_fixed_delta().in_milliseconds(), 2000)) {
			LOG("Connection to server timed out.");
			client.forceful_disconnect();
		}

		{
			auto scope = measure_scope(get_profiler().sending_packets);
			client.send_pending_redundant();
		}

		total_collected_entropy.clear();
	}

	if (!still_downloading) {
		const augs::delta vdt =
			timer.get_stepping_speed_multiplier()
			* session.frame_timer.extract<std::chrono::milliseconds>()
		;
		
		thread_local visible_entities all_visible;
		session.get_visible_entities(all_visible, hypersomnia);

		session.advance_audiovisual_systems(
			extrapolated_hypersomnia, 
			currently_controlled_character,
			all_visible,
			vdt
		);

		auto& renderer = augs::renderer::get_current();
		
		if (swap_buffers) {
			renderer.clear_current_fbo();
		}

		session.view(
			renderer,
			extrapolated_hypersomnia,
			currently_controlled_character,
			all_visible,
			timer.fraction_of_step_until_next_step(extrapolated_hypersomnia.get_fixed_delta()),
			client
		);

		if (swap_buffers) {
			window.swap_buffers();
		}
	}
}