#include <unordered_map>
#include "director_setup.h"

#include <thread>

#include "augs/global_libraries.h"
#include "augs/templates/container_templates.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "game/assets/assets_manager.h"
#include "game/organization/all_component_includes.h"
#include "view/network/step_packaged_for_network.h"
#include "game/transcendental/logic_step.h"
#include "game/organization/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"
#include "game/detail/visible_entities.h"

#include "augs/window_framework/window.h"
#include "application/config_lua_table.h"
#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"

#define LOG_REWINDING 0

using namespace augs::event::keys;

director_setup::recording_boolset director_setup::get_flags() {
	switch (recording_mode) {
	case recording_type::ALL: return { recording_flags::ALLOW_KEYS, recording_flags::ALLOW_MOUSE };
	case recording_type::ONLY_KEYS: return { recording_flags::ALLOW_KEYS };
	case recording_type::ONLY_MOUSE: return { recording_flags::ALLOW_MOUSE };
	default: ensure(false); return {};
	}
}

void director_setup::init(
	game_window& window,
	viewing_session& session
) {
	metas_of_assets = *get_assets_manager().generate_logical_metas_of_assets();

	if (!cosm.load_from_file("save.state")) {
		cosm.set_fixed_delta(session.config.default_tickrate);

		if (session.config.debug_minimal_test_scene) {
			test_scenes::minimal_scene().populate_with_entities(
				cosm,
				metas_of_assets,
				session.get_standard_post_solve()
			);
		}
		else {
			test_scenes::testbed().populate_with_entities(
				cosm,
				metas_of_assets,
				session.get_standard_post_solve()
			);
		}
	}

	characters.acquire_available_characters(cosm);

	input_director_path = session.config.director_input_scene_entropy_path;
	output_director_path = session.config.director_input_scene_entropy_path;

	augs::create_directories(output_director_path);

	director.load_recording_from_file(input_director_path);
	
	set_snapshot_frequency_in_seconds(3.0);

	initial_step_number = cosm.get_total_steps_passed();

	timer.reset_timer();
}

void director_setup::set_snapshot_frequency_in_seconds(const double seconds_between_snapshots) {
	snapshot_frequency_in_steps = static_cast<unsigned>(seconds_between_snapshots / cosm.get_fixed_delta().in_seconds());
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
		if (recording_mode == recording_type::ALL) {
			status_text += format_as_bbcode(L"[color=red]Recording (replacing all)[/color]", white_font);
		}
		else if (recording_mode == recording_type::ONLY_KEYS) {
			status_text += format_as_bbcode(L"[color=red]Recording (replacing keys)[/color]", white_font);
		}
		else if (recording_mode == recording_type::ONLY_MOUSE) {
			status_text += format_as_bbcode(L"[color=red]Recording (replacing mouse)[/color]", white_font);
		}
		else {
			ensure(false);
		}
	}

	status_text += format(typesafe_sprintf(L"\nRequested playing speed: %x", requested_playing_speed), white_font);
	status_text += format(typesafe_sprintf(L"\nStep number: %x", get_step_number(cosm)), white_font);
	status_text += format(typesafe_sprintf(L"\nTime: %x", get_step_number(cosm)*cosm.get_fixed_delta().in_seconds()), white_font);
	status_text += 
		format(
			typesafe_sprintf(
				L"\nControlling entity %x of %x (guid: %x)", 
				characters.current_character_index, 
				characters.characters.size(),
				cosm[characters.get_selected_character()].get_guid()
			), 
		white_font
	);

	if (bookmarked_step != 0) {
		status_text += format(typesafe_sprintf(L"\nBookmarked time: %x", bookmarked_step*cosm.get_fixed_delta().in_seconds()), white_font);
	}

	if (unsaved_changes_exist) {
		status_text += format_as_bbcode(L"\n[color=yellow]Press F7 to save pending changes.[/color]", white_font);
	}

	return status_text;
}

void director_setup::clear_accumulated_inputs() {
	total_collected_entropy.clear();
}

augs::machine_entropy director_setup::control_player(
	game_window& window,
	viewing_session& session
) {
	int advance_steps_forward = 0;

	augs::machine_entropy new_machine_entropy;

	{
		auto scope = measure_scope(get_profiler().local_entropy);
		new_machine_entropy.local = window.collect_entropy();
	}

	process_exit(new_machine_entropy.local);

	for (const auto& raw_input : new_machine_entropy.local) {
		events.apply(raw_input);

		if (raw_input.was_any_key_pressed()) {
			if (raw_input.key == key::F2) {
				current_director_state = director_state::PLAYING;
			}
			if (raw_input.key == key::F3) {
				requested_playing_speed = 0.0;

				if (current_director_state == director_state::RECORDING) {
					recording_mode =
						static_cast<recording_type>((static_cast<int>(recording_mode) + 1) % static_cast<int>(recording_type::COUNT));
				}
				else {
					recording_mode = recording_type::ALL;

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
				requested_playing_speed = 1.0;
				clear_accumulated_inputs();
			}
			if (raw_input.key == key::NUMPAD6) {
				requested_playing_speed = 6.0;
				clear_accumulated_inputs();
			}
			if (raw_input.key == key::NUMPAD9) {
				bookmarked_step = get_step_number(cosm);
				clear_accumulated_inputs();
			}
			if (raw_input.key == key::NUMPAD0) {
				advance_steps_forward = static_cast<long long>(bookmarked_step) - static_cast<long long>(get_step_number(cosm));
				current_director_state = director_state::PLAYING;
				clear_accumulated_inputs();
			}
		}
	}

	session.switch_between_gui_and_back(new_machine_entropy.local);

	session.fetch_gui_events(
		cosm[characters.get_selected_character()],
		new_machine_entropy.local
	);

	//if (should_show_editor_gui()) {
	//	for (const auto l : new_machine_entropy.local) {
	//		session.perform_imgui_pass({ l }, 0.f);
	//	}
	//}

	characters.control_character_selection_numeric(new_machine_entropy.local);

	auto translated = session.config.controls.translate(new_machine_entropy.local);

	session.fetch_session_intents(translated.intents);
	characters.control_character_selection(translated.intents);

	auto new_cosmic_entropy = cosmic_entropy(
		cosm[characters.get_selected_character()],
		translated
	);

	new_cosmic_entropy += session.systems_audiovisual.get<gui_element_system>().get_and_clear_pending_events();

	if (current_director_state == director_state::RECORDING) {
		total_collected_entropy += new_cosmic_entropy;
	}

	const auto current_step = get_step_number(cosm);

	if (advance_steps_forward < 0) {
		/* Trim to zero if we want to rewind too much */
		const unsigned seeked_step = static_cast<unsigned>(-advance_steps_forward) > current_step ? 0 : current_step + advance_steps_forward;
		seek_to_step(seeked_step, session);
	}
	else if (advance_steps_forward > 0) {
		const unsigned seeked_step = current_step + static_cast<unsigned>(advance_steps_forward);
		seek_to_step(seeked_step, session);
	}

	return new_machine_entropy;
}

void director_setup::seek_to_step(
	const unsigned seeked_step,
	viewing_session& session
) {
	const auto previous_snapshot_index = std::min(
		snapshots_for_rewinding.size() - 1,
		seeked_step / snapshot_frequency_in_steps
	);

	if (seeked_step < get_step_number(cosm)) {
		cosm = snapshots_for_rewinding.at(previous_snapshot_index);
	}

	// at this point the seeked_step is either equal or greater than the current

	const auto distance_from_previous_snapshot = seeked_step - previous_snapshot_index * snapshot_frequency_in_steps;
	const auto distance_from_current = seeked_step - get_step_number(cosm);

	if (distance_from_previous_snapshot < distance_from_current) {
		cosm = snapshots_for_rewinding.at(previous_snapshot_index);
	}

	while (get_step_number(cosm) < seeked_step) {
		advance_player_by_single_step(session);
	}
}

void director_setup::push_snapshot_if_needed() {
	const auto current_step = get_step_number(cosm);

	if (current_step % snapshot_frequency_in_steps == 0) {
		const auto snapshot_index = current_step / snapshot_frequency_in_steps;

		const bool valid_snapshot_exists = snapshot_index < snapshots_for_rewinding.size();

		if (!valid_snapshot_exists) {
			snapshots_for_rewinding.push_back(cosm);
		}
	}
}

void director_setup::process(
	game_window& window,
	viewing_session& session
) {
	init(window, session);

	while (!should_quit) {
		sync_config_back(session.config, window);

		const auto screen_size = window.get_screen_size();
		augs::renderer::get_current().resize_fbos(screen_size);
		session.set_screen_size(screen_size);

		const auto entropy = control_player(window, session);

		if (should_show_editor_gui()) {
			session.perform_imgui_pass(
				window,
				entropy.local,
				session.imgui_timer.extract<std::chrono::milliseconds>()
			);
		}

		advance_player(session);
		view(session);
		window.swap_buffers();
	}
}

void director_setup::advance_player_by_single_step(viewing_session& session) {
	cosmic_entropy cosmic_entropy_for_this_advancement;
	const auto current_step = get_step_number(cosm);

	push_snapshot_if_needed();

	if (current_director_state == director_state::PLAYING) {
		guid_mapped_entropy replayed_entropy = director.get_entropy_for_step(current_step);

		cosmic_entropy_for_this_advancement = cosmic_entropy(replayed_entropy, cosm);
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
		
		const auto total_collected_guid_entropy = guid_mapped_entropy(total_collected_entropy, cosm);

		guid_mapped_entropy& entropy_for_this_advancement = director.step_to_entropy[current_step];

		entropy_for_this_advancement.override_transfers_leaving_other_entities(
			cosm,
			*total_collected_guid_entropy.transfer_requests
		);

		const auto flags = get_flags();

		const auto selected_character_guid = cosm[characters.get_selected_character()].get_guid();

		if (flags.test(recording_flags::ALLOW_KEYS)) {
			entropy_for_this_advancement.cast_spells_per_entity.erase(selected_character_guid);
			entropy_for_this_advancement.intents_per_entity.erase(selected_character_guid);

			for (const auto new_spell_requested : total_collected_guid_entropy.cast_spells_per_entity) {
				entropy_for_this_advancement.cast_spells_per_entity[new_spell_requested.first] = new_spell_requested.second;
			}

			for (const auto& new_intents_requested : total_collected_guid_entropy.intents_per_entity) {
				auto new_intents = new_intents_requested.second;
				auto& intents_written_to = entropy_for_this_advancement.intents_per_entity[new_intents_requested.first];
				
				intents_written_to = new_intents;
			}
		}
		
		if (flags.test(recording_flags::ALLOW_MOUSE)) {
			entropy_for_this_advancement.motions_per_entity.erase(selected_character_guid);

			for (const auto& new_motions_requested : total_collected_guid_entropy.motions_per_entity) {
				auto new_motions = new_motions_requested.second;
				auto& motions_written_to = entropy_for_this_advancement.motions_per_entity[new_motions_requested.first];

				motions_written_to = new_motions;
			}
		}

		cosmic_entropy_for_this_advancement = cosmic_entropy(entropy_for_this_advancement, cosm);

		unsaved_changes_exist = true;
	}

	augs::renderer::get_current().clear_logic_lines();

	cosm.advance_deterministic_schemata(
		{ cosmic_entropy_for_this_advancement, metas_of_assets },
		[](auto) {},
		session.get_standard_post_solve()
	);

	total_collected_entropy.clear();

	advance_audiovisuals(session);
}

void director_setup::advance_player(viewing_session& session) {
	auto steps = timer.count_logic_steps_to_perform(cosm.get_fixed_delta());

	const auto speed = requested_playing_speed * basic_playback_speed;

	timer.set_stepping_speed_multiplier(speed);

	const bool is_paused = !(speed > 0.);

	if (is_paused) {
		session.set_interpolation_enabled(false);
		total_collected_entropy.clear();
	}
	else {
		session.set_interpolation_enabled(true);
	}

	if (!steps) {
		advance_audiovisuals(session);
	}

	while (steps--) {
		advance_player_by_single_step(session);
	}
}

thread_local visible_entities all_visible;

void director_setup::advance_audiovisuals(
	viewing_session& session
) {
	session.get_visible_entities(all_visible, cosm);
	
	const augs::delta vdt =
		timer.get_stepping_speed_multiplier()
		* session.frame_timer.extract<std::chrono::milliseconds>()
	;

	session.advance_audiovisual_systems(
		cosm,
		characters.get_selected_character(),
		all_visible,
		vdt
	);
}

void director_setup::view(
	viewing_session& session
) {
	advance_audiovisuals(session);

	auto& renderer = augs::renderer::get_current();
	renderer.clear_current_fbo();

	session.view(
		renderer,
		cosm,
		characters.get_selected_character(),
		all_visible,
		timer.fraction_of_step_until_next_step(cosm.get_fixed_delta()),
		get_status_text()
	);

	renderer.draw_imgui(get_assets_manager());
}

void director_setup::save_unsaved_changes() {
	if (unsaved_changes_exist) {
		director.save_recording_to_file(augs::replace_extension(output_director_path, ".unsaved.ent"));
	}
}

director_setup::~director_setup() {
	save_unsaved_changes();
}

bool director_setup::should_show_editor_gui() const {
	return requested_playing_speed == 0.0;
}
