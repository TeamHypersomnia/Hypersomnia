#include "director_setup.h"

#include <thread>
#include <experimental/filesystem>

#include "augs/global_libraries.h"
#include "augs/templates/container_templates.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "game/bindings/bind_game_and_augs.h"
#include "game/resources/manager.h"
#include "game/resource_setups/all.h"
#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"
#include "game/detail/visible_entities.h"

#include "application/game_window.h"
#include "application/config_lua_table.h"

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

	if (!hypersomnia.load_from_file("save.state")) {
		hypersomnia.set_fixed_delta(cfg.default_tickrate);

		testbed.populate_world_with_entities(
			hypersomnia,
			get_standard_post_solve()
		);
	}

	hypersomnia[testbed.characters[0]].get<components::name>().nickname = ::to_wstring(cfg.nickname);

	if (testbed.characters.size() > 1) {
		hypersomnia[testbed.characters[1]].get<components::name>().nickname = ::to_wstring(cfg.debug_second_nickname);
	}

	input_director_path = cfg.director_input_scene_entropy_path;
	output_director_path = cfg.director_input_scene_entropy_path;

	augs::create_directories(output_director_path);

	director.load_recording_from_file(input_director_path);
	
	set_snapshot_frequency_in_seconds(3.0);

	initial_step_number = hypersomnia.get_total_steps_passed();

	timer.reset_timer();
}

void director_setup::set_snapshot_frequency_in_seconds(const double seconds_between_snapshots) {
	snapshot_frequency_in_steps = static_cast<unsigned>(seconds_between_snapshots / hypersomnia.get_fixed_delta().in_seconds());
	snapshots_for_rewinding.clear();
}

unsigned director_setup::get_step_number(const cosmos& cosm) const {
	ensure(initial_step_number <= cosm.get_total_steps_passed());

	return cosm.get_total_steps_passed() - initial_step_number;
};

augs::gui::text::formatted_string director_setup::get_status_text() const {
	using namespace augs::gui::text;

	const auto white_font = style(assets::font_id::GUI_FONT, white);

	auto status_text = format(L"Welcome to the director setup.", white_font);
	status_text += format(L"\nMode: ", white_font);

	if (current_director_state == director_state::PLAYING) {
		status_text += format(L"Playing", white_font);
	}
	else {
		if (recording_replacement_mode == recording_replacement_type::ALL) {
			status_text += format_as_bbcode(L"[color=red]Recording (replacing all)[/color]", white_font);
		}
		else if (recording_replacement_mode == recording_replacement_type::ONLY_KEYS) {
			status_text += format_as_bbcode(L"[color=red]Recording (replacing keys)[/color]", white_font);
		}
		else if (recording_replacement_mode == recording_replacement_type::ONLY_MOUSE) {
			status_text += format_as_bbcode(L"[color=red]Recording (replacing mouse)[/color]", white_font);
		}
		else {
			ensure(false);
		}
	}

	status_text += format(typesafe_sprintf(L"\nRequested playing speed: %x", requested_playing_speed), white_font);
	status_text += format(typesafe_sprintf(L"\nStep number: %x", get_step_number(hypersomnia)), white_font);
	status_text += format(typesafe_sprintf(L"\nTime: %x", get_step_number(hypersomnia)*hypersomnia.get_fixed_delta().in_seconds()), white_font);
	status_text += 
		format(
			typesafe_sprintf(
				L"\nControlling entity %x of %x (guid: %x)", 
				testbed.current_character_index, 
				testbed.characters.size(),
				hypersomnia[testbed.get_selected_character()].get_guid()
			), 
		white_font
	);

	if (bookmarked_step != 0) {
		status_text += format(typesafe_sprintf(L"\nBookmarked time: %x", bookmarked_step*hypersomnia.get_fixed_delta().in_seconds()), white_font);
	}

	if (unsaved_changes_exist) {
		status_text += format_as_bbcode(L"\n[color=yellow]Press F7 to save pending changes.[/color]", white_font);
	}

	return status_text;
}

void director_setup::clear_accumulated_inputs() {
	total_collected_entropy.clear();
}

void director_setup::control_player(
	const config_lua_table& cfg,
	game_window& window
) {
	int advance_steps_forward = 0;

	augs::machine_entropy new_machine_entropy;

	session.local_entropy_profiler.new_measurement();
	new_machine_entropy.local = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
	session.local_entropy_profiler.end_measurement();

	process_exit_key(new_machine_entropy.local);

	for (const auto& raw_input : new_machine_entropy.local) {
		events.apply(raw_input);

		if (raw_input.was_any_key_pressed()) {
			if (raw_input.key == key::F2) {
				current_director_state = director_state::PLAYING;
			}
			if (raw_input.key == key::F3) {
				requested_playing_speed = 0.0;

				if (current_director_state == director_state::RECORDING) {
					recording_replacement_mode =
						static_cast<recording_replacement_type>((static_cast<int>(recording_replacement_mode) + 1) % static_cast<int>(recording_replacement_type::COUNT));
				}
				else {
					recording_replacement_mode = recording_replacement_type::ALL;

					current_director_state = director_state::RECORDING;
					clear_accumulated_inputs();
				}
			}
			if (raw_input.key == key::F7) {
				director.save_recording_to_file(output_director_path);
				unsaved_changes_exist = false;

				requested_playing_speed = 0.0;
				clear_accumulated_inputs();
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
				requested_playing_speed = 0.0;
				clear_accumulated_inputs();
			}
			if (raw_input.key == key::NUMPAD4) {
				requested_playing_speed = 0.1;
				clear_accumulated_inputs();
			}
			if (raw_input.key == key::NUMPAD5) {
				requested_playing_speed = 1.;
				clear_accumulated_inputs();
			}
			if (raw_input.key == key::NUMPAD6) {
				requested_playing_speed = 6.;
				clear_accumulated_inputs();
			}
			if (raw_input.key == key::NUMPAD9) {
				bookmarked_step = get_step_number(hypersomnia);
				clear_accumulated_inputs();
			}
			if (raw_input.key == key::NUMPAD0) {
				advance_steps_forward = static_cast<long long>(bookmarked_step) - static_cast<long long>(get_step_number(hypersomnia));
				clear_accumulated_inputs();
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

	const auto current_step = get_step_number(hypersomnia);

	if (advance_steps_forward < 0) {
		const unsigned seeked_step = static_cast<unsigned>(-advance_steps_forward) > current_step ? 0 : current_step + advance_steps_forward;
		seek_to_step(seeked_step);
	}
	else if (advance_steps_forward > 0) {
		const unsigned seeked_step = current_step + static_cast<unsigned>(advance_steps_forward);
		seek_to_step(seeked_step);
	}
}

void director_setup::seek_to_step(const unsigned seeked_step) {
	const auto snapshot_index = seeked_step / snapshot_frequency_in_steps;

	if (seeked_step < get_step_number(hypersomnia)) {
		hypersomnia = snapshots_for_rewinding.at(snapshot_index);
	}

	// at this point the seeked_step is either equal or greater than the current

	const auto distance_from_closest_snapshot = seeked_step % snapshot_frequency_in_steps;
	const auto distance_from_current = seeked_step - get_step_number(hypersomnia);

	if (distance_from_closest_snapshot < distance_from_current) {
		hypersomnia = snapshots_for_rewinding.at(snapshot_index);
	}

	while (get_step_number(hypersomnia) < seeked_step) {
		advance_player_by_single_step();
	}
}

void director_setup::push_snapshot_if_needed() {
	const auto current_step = get_step_number(hypersomnia);

	if (current_step % snapshot_frequency_in_steps == 0) {
		const auto snapshot_index = current_step / snapshot_frequency_in_steps;

		const bool valid_snapshot_exists = snapshot_index < snapshots_for_rewinding.size();

		if (!valid_snapshot_exists) {
			snapshots_for_rewinding.push_back(hypersomnia);
		}
	}
}

void director_setup::process(
	const config_lua_table& cfg, 
	game_window& window
) {
	init(cfg, window);

	while (!should_quit) {
		control_player(cfg, window);
		advance_player();
		view(cfg);
		window.swap_buffers();
	}
}

void director_setup::advance_player_by_single_step() {
	cosmic_entropy cosmic_entropy_for_this_advancement;
	const auto current_step = get_step_number(hypersomnia);

	push_snapshot_if_needed();

	if (current_director_state == director_state::PLAYING) {
		guid_mapped_entropy replayed_entropy = director.get_entropy_for_step(current_step);

		cosmic_entropy_for_this_advancement = cosmic_entropy(replayed_entropy, hypersomnia);
	}
	else if (current_director_state == director_state::RECORDING) {
		const auto next_snapshot_index = 1 + current_step / snapshot_frequency_in_steps;
		const bool outdated_snapshots_to_delete_exist = next_snapshot_index < snapshots_for_rewinding.size();

		if (outdated_snapshots_to_delete_exist) {
			snapshots_for_rewinding.erase(
				snapshots_for_rewinding.begin() + next_snapshot_index,
				snapshots_for_rewinding.end()
			);
		}

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
		[](auto) {},
		get_standard_post_solve()
	);

	total_collected_entropy.clear();
}

void director_setup::advance_player() {
	auto steps = timer.count_logic_steps_to_perform(hypersomnia.get_fixed_delta());

	session.set_interpolation_enabled(requested_playing_speed > 0.);

	while (steps--) {
		advance_player_by_single_step();
	}
}

void director_setup::view(
	const config_lua_table& cfg
) {
	static thread_local visible_entities all_visible;
	session.get_visible_entities(all_visible, hypersomnia);

	const auto vdt = session.frame_timer.extract_variable_delta(hypersomnia.get_fixed_delta(), timer);

	session.advance_audiovisual_systems(
		hypersomnia,
		testbed.get_selected_character(),
		all_visible,
		vdt
	);

	auto& renderer = augs::renderer::get_current();
	renderer.clear_current_fbo();

	session.view(
		cfg,
		renderer,
		hypersomnia,
		testbed.get_selected_character(),
		all_visible,
		timer.fraction_of_step_until_next_step(hypersomnia.get_fixed_delta()),
		get_status_text()
	);
}

void director_setup::save_unsaved_changes() {
	if (unsaved_changes_exist) {
		director.save_recording_to_file(fs::path(output_director_path).replace_extension(".unsaved.ent").string());
	}
}

director_setup::~director_setup() {
	save_unsaved_changes();
}
