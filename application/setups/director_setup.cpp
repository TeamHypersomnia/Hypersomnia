#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/visible_entities.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "director_setup.h"

#include "augs/templates/container_templates.h"
#include "application/config_lua_table.h"

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

#define LOG_REWINDING 0

using namespace augs::window::event::keys;

void director_setup::init(
	const config_lua_table& cfg, 
	game_window& window
) {
	const vec2i screen_size = vec2i(window.get_screen_size());

	session.reserve_caches_for_entities(3000);
	session.set_screen_size(screen_size);
	session.systems_audiovisual.get<interpolation_system>().interpolation_speed = cfg.interpolation_speed;
	session.set_master_gain(cfg.sound_effects_volume);

	session.configure_input();
	
	testbed.debug_var = cfg.debug_var;

	auto standard_post_solve = [this](const const_logic_step step) {
		session.standard_audiovisual_post_solve(step);
	};

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(cfg.default_tickrate);

		testbed.populate_world_with_entities(
			hypersomnia,
			standard_post_solve
		);
	}

	hypersomnia[testbed.characters[0]].get<components::name>().nickname = ::to_wstring(cfg.nickname);

	if (testbed.characters.size() > 1) {
		hypersomnia[testbed.characters[1]].get<components::name>().nickname = ::to_wstring(cfg.debug_second_nickname);
	}

	augs::create_directories(cfg.director_scenario_path);

	input_director_path = cfg.director_scenario_path;
	output_director_path = cfg.director_scenario_path;

	director.load_recording_from_file(input_director_path);
	
	set_snapshot_frequency_in_seconds(3.0);

	initial_step_number = hypersomnia.get_total_steps_passed();

	timer.reset_timer();
}

void director_setup::set_snapshot_frequency_in_seconds(const double seconds_between_snapshots) {
	snapshot_frequency_in_steps = static_cast<unsigned>(seconds_between_snapshots / hypersomnia.get_fixed_delta().in_seconds());
}

unsigned director_setup::get_step_number(const cosmos& cosm) const {
	ensure(initial_step_number <= cosm.get_total_steps_passed());

	return cosm.get_total_steps_passed() - initial_step_number;
};

void director_setup::process(const config_lua_table& cfg, game_window& window) {
	init(cfg, window);

	auto standard_post_solve = [this](const const_logic_step step) {
		session.standard_audiovisual_post_solve(step);
	};

	int advance_steps_forward = 0;

	while (!should_quit) {
		{
			augs::machine_entropy new_machine_entropy;

			session.local_entropy_profiler.new_measurement();
			new_machine_entropy.local = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
			session.local_entropy_profiler.end_measurement();

			process_exit_key(new_machine_entropy.local);

			const auto clear_all_inputs = [&]() {
				total_collected_entropy = cosmic_entropy();
			};

			for (const auto& raw_input : new_machine_entropy.local) {
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
							clear_all_inputs();
						}
					}
					if (raw_input.key == key::F7) {
						director.save_recording_to_file(output_director_path);
						unsaved_changes_exist = false;

						requested_playing_speed = 0.f;
						clear_all_inputs();
					}

					if (raw_input.key == key::NUMPAD1) {
						current_director_state = director_state::PLAYING;
						advance_steps_forward = -1;

						if (events.is_set(key::LCTRL)) {
							advance_steps_forward *= 10;
						}
					}
					if (raw_input.key == key::NUMPAD2) {
						current_director_state = director_state::PLAYING;
						advance_steps_forward = 1;

						if (events.is_set(key::LCTRL)) {
							advance_steps_forward *= 10;
						}
					}
					if (raw_input.key == key::NUMPAD3) {
						requested_playing_speed = 0.f;
						clear_all_inputs();
					}
					if (raw_input.key == key::NUMPAD4) {
						requested_playing_speed = 0.1f;
						clear_all_inputs();
					}
					if (raw_input.key == key::NUMPAD5) {
						requested_playing_speed = 1.f;
						clear_all_inputs();
					}
					if (raw_input.key == key::NUMPAD6) {
						requested_playing_speed = 6.f;
						clear_all_inputs();
					}
					if (raw_input.key == key::NUMPAD9) {
						bookmarked_step = get_step_number(hypersomnia);
						clear_all_inputs();
					}
					if (raw_input.key == key::NUMPAD0) {
						advance_steps_forward = static_cast<long long>(bookmarked_step) - static_cast<long long>(get_step_number(hypersomnia));
						clear_all_inputs();
					}
				}
			}

			session.switch_between_gui_and_back(new_machine_entropy.local);

			session.control_gui_and_remove_fetched_events(
				hypersomnia[testbed.get_selected_character()],
				new_machine_entropy.local
			);

			auto new_intents = session.context.to_key_and_mouse_intents(new_machine_entropy.local);

			session.control_and_remove_fetched_intents(new_intents);
			testbed.control_character_selection(new_intents);

			auto new_cosmic_entropy = cosmic_entropy(
				hypersomnia[testbed.get_selected_character()],
				new_intents
			);

			new_cosmic_entropy += session.systems_audiovisual.get<gui_element_system>().get_and_clear_pending_events();

			if (current_director_state == director_state::RECORDING) {
				total_collected_entropy += new_cosmic_entropy;
			}

			timer.set_stepping_speed_multiplier(requested_playing_speed);
		}

		if (advance_steps_forward < 0) {
			const auto current_step = get_step_number(hypersomnia);
			const auto rewound_step = static_cast<unsigned>(-advance_steps_forward) > current_step ? 0 : current_step + advance_steps_forward;

#if LOG_REWINDING
			LOG("Current step: %x\nRewound step: %x", current_step, rewound_step);
#endif

			if (rewound_step < current_step) {
				const size_t resimulated_cosmos_index = rewound_step / snapshot_frequency_in_steps;
				
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
					const auto cosmic_entropy_for_this_advancement = cosmic_entropy(replayed_entropy, hypersomnia);

					hypersomnia.advance_deterministic_schemata(
						cosmic_entropy_for_this_advancement, 
						[](auto){},
						standard_post_solve
					);
				}
			}

			advance_steps_forward = 0;
		}

		auto new_steps_forward = timer.count_logic_steps_to_perform(hypersomnia.get_fixed_delta());
		
		new_steps_forward += advance_steps_forward;
		advance_steps_forward = 0;

		session.set_interpolation_enabled(requested_playing_speed > 0.f);

		while (new_steps_forward--) {
			cosmic_entropy cosmic_entropy_for_this_advancement;
			const auto current_step = get_step_number(hypersomnia);

			if (current_step % snapshot_frequency_in_steps == 0) {
				snapshots_for_rewinding.push_back(hypersomnia);
			}

			if (current_director_state == director_state::PLAYING) {
				guid_mapped_entropy replayed_entropy = director.get_entropy_for_step(current_step);

				cosmic_entropy_for_this_advancement = cosmic_entropy(replayed_entropy, hypersomnia);
			}
			else if (current_director_state == director_state::RECORDING) {
				const auto total_collected_guid_entropy = guid_mapped_entropy(total_collected_entropy, hypersomnia);

				guid_mapped_entropy& entropy_for_this_advancement = director.step_to_entropy[current_step];

				entropy_for_this_advancement.override_transfers_leaving_other_entities(
					hypersomnia, 
					*total_collected_guid_entropy.transfer_requests
				);

				if (recording_replacement_mode != recording_replacement_type::ONLY_MOUSE) {
					entropy_for_this_advancement.cast_spells->erase(hypersomnia[testbed.get_selected_character()].get_guid());
					
					for (const auto new_spell_requested : total_collected_guid_entropy.cast_spells) {
						entropy_for_this_advancement.cast_spells[new_spell_requested.first] = new_spell_requested.second;
					}
				}

				for (const auto& new_intents_requested : total_collected_guid_entropy.intents_per_entity) {
					auto new_intents = new_intents_requested.second;
					auto& intents_written_to = entropy_for_this_advancement.intents_per_entity[new_intents_requested.first];

					auto mouse_remover = [](const auto& k) { return k.uses_mouse_motion(); };
					auto key_remover = [](const auto& k) { return !k.uses_mouse_motion(); };

					if (recording_replacement_mode == recording_replacement_type::ALL) {
						intents_written_to = new_intents;
					}
					else if (recording_replacement_mode == recording_replacement_type::ONLY_KEYS) {
						erase_remove(intents_written_to, key_remover);
						erase_remove(new_intents, mouse_remover);

						concatenate(intents_written_to, new_intents);
					}
					else if (recording_replacement_mode == recording_replacement_type::ONLY_MOUSE) {
						erase_remove(intents_written_to, mouse_remover);
						erase_remove(new_intents, key_remover);

						concatenate(intents_written_to, new_intents);
					}
					else {
						ensure(false);
					}

				}

				cosmic_entropy_for_this_advancement = cosmic_entropy(entropy_for_this_advancement, hypersomnia);

				unsaved_changes_exist = true;
			}

			augs::renderer::get_current().clear_logic_lines();

			hypersomnia.advance_deterministic_schemata(
				cosmic_entropy_for_this_advancement, 
				[](auto){},
				standard_post_solve
			);
			
			total_collected_entropy = cosmic_entropy();
		}

		static thread_local visible_entities all_visible;
		session.get_visible_entities(all_visible, hypersomnia);

		const auto vdt = session.frame_timer.extract_variable_delta(hypersomnia.get_fixed_delta(), timer);

		session.advance_audiovisual_systems(
			hypersomnia, 
			testbed.get_selected_character(),
			all_visible,
			vdt
		);

		using namespace augs::gui::text;

		const auto white_font = style(assets::font_id::GUI_FONT, white);

		auto director_text = format(L"Welcome to the director setup.", white_font);
		director_text += format(L"\nMode: ", white_font); 
		
		if (current_director_state == director_state::PLAYING) {
			director_text += format(L"Playing", white_font);
		}
		else {
			if (recording_replacement_mode == recording_replacement_type::ALL) {
				director_text += format_as_bbcode(L"[color=red]Recording (replacing all)[/color]", white_font);
			}
			else if (recording_replacement_mode == recording_replacement_type::ONLY_KEYS) {
				director_text += format_as_bbcode(L"[color=red]Recording (replacing keys)[/color]", white_font);
			}
			else if (recording_replacement_mode == recording_replacement_type::ONLY_MOUSE) {
				director_text += format_as_bbcode(L"[color=red]Recording (replacing mouse)[/color]", white_font);
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
			director_text += format_as_bbcode(L"\n[color=yellow]Press F7 to save pending changes.[/color]", white_font);
		}

		auto& renderer = augs::renderer::get_current();
		renderer.clear_current_fbo();

		session.view(
			cfg,
			renderer, 
			hypersomnia, 
			testbed.get_selected_character(), 
			all_visible,
			timer.fraction_of_step_until_next_step(hypersomnia.get_fixed_delta()), 
			director_text
		);

		window.swap_buffers();
	}

	if (unsaved_changes_exist) {
		director.save_recording_to_file(fs::path(output_director_path).replace_extension(".unsaved.ent").string());
	}
}