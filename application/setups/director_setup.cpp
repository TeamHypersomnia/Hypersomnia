#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_managers/testbed.h"
#include "game/scene_managers/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/viewing_session.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/transcendental/types_specification/all_messages_includes.h"

#include "augs/misc/machine_entropy_player.h"

#include "augs/filesystem/file.h"
#include "director_setup.h"

#include "augs/templates/container_templates.h"

#define LOG_REWINDING 0

using namespace augs::window::event::keys;

void director_setup::process(const config_lua_table& cfg, game_window& window) {
	const vec2i screen_size = vec2i(window.get_screen_size());

	cosmos hypersomnia(3000);

	augs::window::event::state events;
	augs::machine_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = augs::fixed_delta_timer(5);

	scene_managers::testbed testbed;
	testbed.debug_var = cfg.debug_var;

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(cfg.tickrate);
		testbed.populate_world_with_entities(hypersomnia, screen_size);
	}

	viewing_session session;
	session.reserve_caches_for_entities(3000);
	session.camera.configure_size(screen_size);
	session.systems_audiovisual.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;
	session.set_interpolation_enabled(false);
	session.set_master_gain(cfg.sound_effects_volume);

	session.configure_input();

	const std::string input_director_file = cfg.director_scenario_filename;
	const std::string output_director_file = cfg.director_scenario_filename;

	cosmic_movie_director director;
	director.load_recording_from_file(input_director_file);

	enum class director_state {
		PLAYING,
		RECORDING
	} current_director_state = director_state::PLAYING;

	timer.reset_timer();

	int advance_steps_forward = 0;

	float requested_playing_speed = 0.f;

	bool unsaved_changes_exist = false;

	enum class recording_replacement_type {
		ALL,
		ONLY_KEYS,
		ONLY_MOUSE,

		COUNT
	};

	auto recording_replacement_mode = recording_replacement_type::ALL;

	std::vector<cosmos> snapshots_for_rewinding;
	
	const auto initial_step_number = hypersomnia.get_total_steps_passed();

	const double seconds_between_snapshots = 3.0;
	const auto steps_between_snapshots = static_cast<unsigned>(seconds_between_snapshots / hypersomnia.get_fixed_delta().in_seconds());

	unsigned bookmarked_step = 0;

	const auto get_step_number = [initial_step_number](const cosmos& cosm) {
		ensure(initial_step_number <= cosm.get_total_steps_passed());

		return cosm.get_total_steps_passed() - initial_step_number;
	};

	LOG("Seconds between rewind snapshots: %x", seconds_between_snapshots);
	LOG("Steps between rewind snapshots: %x", steps_between_snapshots);

	while (!should_quit) {
		{
			augs::machine_entropy new_entropy;

			session.local_entropy_profiler.new_measurement();
			new_entropy.local = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
			session.local_entropy_profiler.end_measurement();
			
			session.control(new_entropy);

			process_exit_key(new_entropy.local);

			if (current_director_state == director_state::RECORDING) {
				total_collected_entropy += new_entropy;
			}

			for (const auto& raw_input : new_entropy.local) {
				events.apply(raw_input);

				if (raw_input.was_any_key_pressed()) {
					if (raw_input.key == key::F2) {
						current_director_state = director_state::PLAYING;
					}
					if (raw_input.key == key::F3) {
						requested_playing_speed = 0.f;

						if (current_director_state == director_state::RECORDING) {
							recording_replacement_mode = 
								static_cast<recording_replacement_type>((static_cast<int>(recording_replacement_mode) + 1) % static_cast<int>(recording_replacement_type::COUNT));
						}
						else {
							recording_replacement_mode = recording_replacement_type::ALL;

							current_director_state = director_state::RECORDING;
							total_collected_entropy.clear();
						}
					}
					if (raw_input.key == key::F7) {
						director.save_recording_to_file(output_director_file);
						unsaved_changes_exist = false;
					}

					if (raw_input.key == key::_1) {
						advance_steps_forward = -1;

						if (events.is_set(key::LCTRL)) {
							advance_steps_forward *= 10;
						}
					}
					if (raw_input.key == key::_2) {
						advance_steps_forward = 1;

						if (events.is_set(key::LCTRL)) {
							advance_steps_forward *= 10;
						}
					}
					if (raw_input.key == key::_3) {
						requested_playing_speed = 0.f;
						total_collected_entropy.clear();
					}
					if (raw_input.key == key::_4) {
						requested_playing_speed = 0.1f;
						total_collected_entropy.clear();
					}
					if (raw_input.key == key::_5) {
						requested_playing_speed = 1.f;
						total_collected_entropy.clear();
					}
					if (raw_input.key == key::_6) {
						requested_playing_speed = 6.f;
						total_collected_entropy.clear();
					}
					if (raw_input.key == key::_9) {
						bookmarked_step = get_step_number(hypersomnia);
						total_collected_entropy.clear();
					}
					if (raw_input.key == key::_0) {
						advance_steps_forward = static_cast<long long>(bookmarked_step) - static_cast<long long>(get_step_number(hypersomnia));
						total_collected_entropy.clear();
					}
				}
			}

			testbed.control_character_selection(new_entropy.local);

			timer.set_stepping_speed_multiplier(requested_playing_speed);
		}

		if (advance_steps_forward < 0) {
			const auto current_step = get_step_number(hypersomnia);
			const auto rewound_step = static_cast<unsigned>(-advance_steps_forward) > current_step ? 0 : current_step + advance_steps_forward;

#if LOG_REWINDING
			LOG("Current step: %x\nRewound step: %x", current_step, rewound_step);
#endif

			if (rewound_step < current_step) {
				const size_t resimulated_cosmos_index = rewound_step / steps_between_snapshots;
				
#if LOG_REWINDING
				LOG_NVPS(snapshots_for_rewinding.size());
				LOG_NVPS(resimulated_cosmos_index);
#endif

				if (snapshots_for_rewinding.size() > 1 && resimulated_cosmos_index < snapshots_for_rewinding.size() - 1) {
					snapshots_for_rewinding.erase(snapshots_for_rewinding.begin() + resimulated_cosmos_index + 1, snapshots_for_rewinding.end());
				}

				hypersomnia = snapshots_for_rewinding.at(resimulated_cosmos_index);
				
#if LOG_REWINDING
				LOG("Resimulated hypersomnia step: %x", get_step_number(hypersomnia));
#endif

				while (get_step_number(hypersomnia) < rewound_step) {
					const guid_mapped_entropy replayed_entropy = director.get_entropy_for_step(get_step_number(hypersomnia));
					const cosmic_entropy cosmic_entropy_for_this_advancement = cosmic_entropy(replayed_entropy, hypersomnia);

					hypersomnia.advance_deterministic_schemata(cosmic_entropy_for_this_advancement, [](auto) {},
						[this, &session](const const_logic_step& step) {
							session.acquire_game_events_for_hud(step);
						}
					);
				}
			}

			advance_steps_forward = 0;
		}

		auto steps = timer.count_logic_steps_to_perform(hypersomnia.get_fixed_delta());
		
		steps += advance_steps_forward;
		advance_steps_forward = 0;

		const auto selected_character = hypersomnia[testbed.get_selected_character()];

		while (steps--) {
			cosmic_entropy cosmic_entropy_for_this_advancement;
			const auto current_step = get_step_number(hypersomnia);

			if (current_step % steps_between_snapshots == 0) {
				snapshots_for_rewinding.push_back(hypersomnia);
			}

			if (current_director_state == director_state::PLAYING) {
				guid_mapped_entropy replayed_entropy = director.get_entropy_for_step(current_step);

				cosmic_entropy_for_this_advancement = cosmic_entropy(replayed_entropy, hypersomnia);
			}
			else if (current_director_state == director_state::RECORDING) {
				guid_mapped_entropy& recorded_entropy = director.step_to_entropy[current_step];

				auto& target_intents_from_recording = recorded_entropy.entropy_per_entity[selected_character.get_guid()];
				auto new_intents = make_intents_for_entity(selected_character, total_collected_entropy.local, session.context);

				auto mouse_remover = [](const auto& k) { return k.uses_mouse_motion(); };
				auto key_remover = [](const auto& k) { return !k.uses_mouse_motion(); };

				if (recording_replacement_mode == recording_replacement_type::ALL) {
					target_intents_from_recording = new_intents;
				}
				else if (recording_replacement_mode == recording_replacement_type::ONLY_KEYS) {
					erase_remove(target_intents_from_recording, key_remover);
					erase_remove(new_intents, mouse_remover);
					
					concatenate(target_intents_from_recording, new_intents);
				}
				else if (recording_replacement_mode == recording_replacement_type::ONLY_MOUSE) {
					erase_remove(target_intents_from_recording, mouse_remover);
					erase_remove(new_intents, key_remover);

					concatenate(target_intents_from_recording, new_intents);
				}
				else {
					ensure(false);
				}
				
				cosmic_entropy_for_this_advancement = cosmic_entropy(recorded_entropy, hypersomnia);

				unsaved_changes_exist = true;
			}

			augs::renderer::get_current().clear_logic_lines();

			hypersomnia.advance_deterministic_schemata(cosmic_entropy_for_this_advancement, [](auto) {},
					[this, &session](const const_logic_step& step) {
					session.acquire_game_events_for_hud(step);
				}
			);

			session.resample_state_for_audiovisuals(hypersomnia);
			
			total_collected_entropy.clear();
		}

		const auto vdt = session.frame_timer.extract_variable_delta(hypersomnia.get_fixed_delta(), timer);

		session.advance_audiovisual_systems(hypersomnia, testbed.get_selected_character(), vdt);

		using namespace augs::gui::text;

		const auto white_font = style(assets::font_id::GUI_FONT, white);

		auto director_text = format(L"Welcome to the director setup.", white_font);
		director_text += format(L"\nMode: ", white_font); 
		
		if (current_director_state == director_state::PLAYING) {
			director_text += format(L"Playing", white_font);
		}
		else {
			if (recording_replacement_mode == recording_replacement_type::ALL) {
				director_text += simple_bbcode(L"[color=red]Recording (replacing all)[/color]", white_font);
			}
			else if (recording_replacement_mode == recording_replacement_type::ONLY_KEYS) {
				director_text += simple_bbcode(L"[color=red]Recording (replacing keys)[/color]", white_font);
			}
			else if (recording_replacement_mode == recording_replacement_type::ONLY_MOUSE) {
				director_text += simple_bbcode(L"[color=red]Recording (replacing mouse)[/color]", white_font);
			}
			else {
				ensure(false);
			}
		}

		director_text += format(typesafe_sprintf(L"\nRequested playing speed: %x", requested_playing_speed), white_font);
		director_text += format(typesafe_sprintf(L"\nStep number: %x", get_step_number(hypersomnia)), white_font);
		director_text += format(typesafe_sprintf(L"\nTime: %x", get_step_number(hypersomnia)*hypersomnia.get_fixed_delta().in_seconds()), white_font);
		director_text += format(typesafe_sprintf(L"\nControlling entity %x of %x", testbed.current_character_index, testbed.characters.size()), white_font);

		if (bookmarked_step != 0) {
			director_text += format(typesafe_sprintf(L"\nBookmarked time: %x", bookmarked_step*hypersomnia.get_fixed_delta().in_seconds()), white_font);
		}

		if (unsaved_changes_exist) {
			director_text += simple_bbcode(L"\n[color=yellow]Press F7 to save pending changes.[/color]", white_font);
		}

		auto& renderer = augs::renderer::get_current();
		renderer.clear_current_fbo();

		session.view(renderer, hypersomnia, testbed.get_selected_character(), vdt, director_text);

		window.swap_buffers();
	}

	if (unsaved_changes_exist) {
		director.save_recording_to_file(output_director_file + ".unsaved.ent");
	}
}