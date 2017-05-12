#include <thread>
#include "choreographic_structs.h"

#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/assets/assets_manager.h"

#include "game/test_scenes/testbed.h"
#include "game/test_scenes/one_entity.h"
#include "game/test_scenes/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/view/viewing_session.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

#include "augs/misc/templated_readwrite.h"

#include "augs/templates/string_templates.h"
#include "augs/filesystem/file.h"
#include "choreographic_setup.h"
#include "augs/tweaker.h"
#include "game/detail/visible_entities.h"
#include "application/config_lua_table.h"

#include "augs/misc/trivial_variant.h"

#include "application/setups/director_setup.h"

#include "generated/introspectors.h"
#include "augs/misc/parsing_utils.h"
#include "augs/audio/sound_samples_from_file.h"

typedef augs::trivial_variant<
	play_scene,
	play_sound,
	focus_guid,
	focus_index,
	set_sfx_gain
> choreographic_command_variant;

using namespace augs::window::event::keys;

void choreographic_setup::process(
	const config_lua_table& cfg,
	game_window& window,
	viewing_session& session
) {
	session.show_profile_details = false;
	session.reserve_caches_for_entities(3000);

	const auto lines = augs::get_file_lines(cfg.choreographic_input_scenario_path);
	size_t current_line = 0;

	ensure(lines[current_line] == "resources:");

	std::unordered_map<resource_id_type, augs::single_sound_buffer> preloaded_sounds;

	struct preloaded_scene {
		director_setup scene;
		std::vector<speed_change> speed_changes;
	};

	std::unordered_map<resource_id_type, preloaded_scene> preloaded_scenes;

	++current_line;

	auto get_line_until = augs::make_get_line_until(lines, current_line);

	while (get_line_until("scene_timings:")) {
		std::istringstream in(lines[current_line]);

		std::string type;
		resource_id_type id;
		std::string path;

		in >> type >> id;
		getline(in, path);

		path = path.substr(path.find_first_not_of(" "));

		if (type == "sound") {
			preloaded_sounds[id].set_data(augs::get_sound_samples_from_file(path));
		}
		else if (type == "scene") {
			auto scene_cfg = cfg;
			scene_cfg.director_input_scene_entropy_path = path;

			auto& scene = preloaded_scenes[id].scene;

			viewing_session dummy;
			scene.init(scene_cfg, window, dummy);
			scene.requested_playing_speed = 1.0;
		}
		else {
			ensure(false && "Unknown resource type!");
		}

		++current_line;
	}

	while (get_line_until("commands:")) {
		std::istringstream in(lines[current_line]);

		resource_id_type id;
		in >> id;

		speed_change change;

		augs::read_members_from_istream(in, change);

		preloaded_scenes[id].speed_changes.push_back(change);

		++current_line;
	}

	for (auto& p : preloaded_scenes) {
		std::stable_sort(
			p.second.speed_changes.begin(), 
			p.second.speed_changes.end(), 
			[](const auto& a, const auto& b){
				return a.at_time < b.at_time;
			}
		);
	}

	std::vector<choreographic_command_variant> events;

	double current_start_time_offset_for_commands = 0.0;

	while (get_line_until()) {
		std::istringstream in(lines[current_line]);
		
		std::string command_name;

		in >> command_name;

		if (command_name == "add_time_offset") {
			double offset = 0.0;
			in >> offset;

			current_start_time_offset_for_commands += offset;
		}

		else if (command_name == "set_time_offset") {
			double offset = 0.0;
			in >> offset;

			current_start_time_offset_for_commands = offset;
		}

		else {
			for_each_through_std_get(choreographic_command_variant::types_tuple(), [&](auto dummy) {
				typedef decltype(dummy) command_type;

				if ("struct " + command_name == typeid(command_type).name()) {
					command_type new_command;
					augs::read_members_from_istream(in, new_command);

					new_command.at_time += current_start_time_offset_for_commands;

					events.push_back(new_command);
				}
			});
		}

		++current_line;
	}

	auto get_start_time = [&](const choreographic_command_variant& a) {
		return a.call([&](const auto& r) { return r.at_time; });
	};

	std::vector<augs::sound_source> sources;

	auto next_played_event_index = 0u;
	int currently_played_scene_index = -1;

	std::stable_sort(
		events.begin(),
		events.end(),
		[&](
			const choreographic_command_variant a, 
			const choreographic_command_variant b
		) {
			return get_start_time(a) < get_start_time(b);
		}
	);

	augs::timer player_timer;
	double player_time = 0.0;
	double current_playback_speed = 1.0;

	auto set_playback_speed_globally = [&](const double new_speed) {
		current_playback_speed = new_speed;

		for (auto& s : sources) {
			s.set_pitch(new_speed);
		}

		for (auto& p : preloaded_scenes) {
			p.second.scene.basic_playback_speed = new_speed;
		}
	};

	while (!should_quit) {
		player_time += player_timer.extract<std::chrono::seconds>() * current_playback_speed;

		if (next_played_event_index < events.size()) {
			const auto& next_event = events[next_played_event_index];

			const auto start_time = get_start_time(next_event);

			if (start_time <= player_time) {
				if (next_event.is<play_scene>()) {
					auto& e = next_event.get<play_scene>();

					currently_played_scene_index = e.id;
				}

				else if (next_event.is<play_sound>()) {
					auto& e = next_event.get<play_sound>();

					augs::sound_source src;
					src.bind_buffer(preloaded_sounds[e.id]);
					src.set_direct_channels(true);
					src.set_pitch(current_playback_speed);
					src.play();

					sources.emplace_back(std::move(src));
				}

				else if (next_event.is<focus_guid>()) {
					auto& e = next_event.get<focus_guid>();

					ensure(currently_played_scene_index != -1);

					auto& scene = preloaded_scenes[currently_played_scene_index].scene;
					scene.testbed.select_character(scene.hypersomnia[e.guid].get_id());
				}

				else if (next_event.is<focus_index>()) {
					auto& e = next_event.get<focus_index>();

					ensure(currently_played_scene_index != -1);

					auto& scene = preloaded_scenes[currently_played_scene_index].scene;
					scene.testbed.select_character(scene.testbed.characters[e.index]);
				}

				else if (next_event.is<set_sfx_gain>()) {
					auto& e = next_event.get<set_sfx_gain>();
					session.set_master_gain(e.gain);
				}

				++next_played_event_index;
			}
		}

		augs::machine_entropy new_machine_entropy;
		new_machine_entropy.local = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
		process_exit_key(new_machine_entropy.local);

		session.switch_between_gui_and_back(new_machine_entropy.local);


		for (const auto& raw_input : new_machine_entropy.local) {
			if (raw_input.was_any_key_pressed()) {
				if (raw_input.key == key::NUMPAD4) {
					set_playback_speed_globally(0.1);
				}
				if (raw_input.key == key::NUMPAD5) {
					set_playback_speed_globally(1.0);
				}
				if (raw_input.key == key::NUMPAD6) {
					set_playback_speed_globally(6.0);
				}
			}
		}

		if (currently_played_scene_index != -1) {
			auto& scene = preloaded_scenes[currently_played_scene_index];
			
			session.control_gui_and_remove_fetched_events(
				scene.scene.hypersomnia[scene.scene.testbed.get_selected_character()],
				new_machine_entropy.local
			);

			const auto current_scene_time = scene.scene.hypersomnia.get_total_time_passed_in_seconds();

			scene.scene.requested_playing_speed = 1.0;

			for (const auto& t : scene.speed_changes) {
				if (current_scene_time >= t.at_time && current_scene_time < t.to_time) {
					scene.scene.requested_playing_speed = t.speed_multiplier;
					break;
				}
			}

			scene.scene.advance_player(session);
			scene.scene.view(cfg, session);
		}
		
		session.systems_audiovisual.get<gui_element_system>().get_and_clear_pending_events();

		using namespace augs::gui::text;

		auto& renderer = augs::renderer::get_current();

		session.draw_text_at_left_top(
			renderer,
			format(
				typesafe_sprintf(L"View time: %x\nPlayback speed: %x", player_time, current_playback_speed),

				style(
					assets::font_id::GUI_FONT,
					rgba(255, 255, 255, 150)
				)
			)
		);

		renderer.call_triangles();
		renderer.clear_triangles();

		window.swap_buffers();
	}
}