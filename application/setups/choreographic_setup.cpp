#include <thread>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/global_libraries.h"
#include "application/game_window.h"

#include "game/resources/manager.h"

#include "game/scene_builders/testbed.h"
#include "game/scene_builders/one_entity.h"
#include "game/resource_setups/all.h"

#include "game/transcendental/types_specification/all_component_includes.h"
#include "game/view/viewing_session.h"
#include "game/transcendental/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

#include "augs/misc/debug_entropy_player.h"

#include "augs/templates/string_templates.h"
#include "augs/filesystem/file.h"
#include "choreographic_setup.h"
#include "augs/tweaker.h"
#include "game/detail/visible_entities.h"
#include "application/config_lua_table.h"

#include "augs/misc/trivial_variant.h"

#include "application/setups/director_setup.h"

#include "generated_introspectors.h"

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
	game_window& window
) {
	const auto lines = augs::get_file_lines(cfg.choreographic_input_scenario_path);
	size_t current_line = 0;

	ensure(lines[current_line] == "resources:");

	std::unordered_map<resource_id_type, augs::single_sound_buffer> preloaded_sounds;
	std::unordered_map<resource_id_type, director_setup> preloaded_scenes;

	++current_line;

	while (lines[current_line] != "commands:") {
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

			preloaded_scenes[id].init(scene_cfg, window);
			preloaded_scenes[id].session.show_profile_details = false;
		}
		else {
			ensure(false && "Unknown resource type!");
		}

		++current_line;
	}

	++current_line;

	std::vector<choreographic_command_variant> events;

	while (current_line < lines.size()) {
		std::istringstream in(lines[current_line]);
		
		std::string command_name;

		in >> command_name;

		for_each_type_in_list(choreographic_command_variant(), [&](auto dummy) {
			typedef decltype(dummy) command_type;

			if ("struct " + command_name == typeid(command_type).name()) {
				command_type new_command;

				augs::introspect_recursive<
					bind_types_t<can_stream_right, std::istringstream>,
					always_recurse,
					stop_recursion_if_valid
				>(
					[&](auto, auto& member) {
						in >> member;
					},
					new_command
				);

				events.push_back(new_command);
			}
		});

		++current_line;
	}

	auto get_start_time = [&](const choreographic_command_variant& a) {
		return a.call([&](auto& r) { return r.at_time; });
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

	augs::timer player_time;

	while (!should_quit) {
		if (next_played_event_index < events.size()) {
			const auto& next_event = events[next_played_event_index];

			const auto start_time = get_start_time(next_event);

			if (start_time <= player_time.get<std::chrono::seconds>()) {
				if (next_event.is<play_scene>()) {
					auto& e = next_event.get<play_scene>();

					currently_played_scene_index = e.id;
				}

				else if (next_event.is<play_sound>()) {
					auto& e = next_event.get<play_sound>();

					augs::sound_source src;
					src.bind_buffer(preloaded_sounds[e.id]);
					src.set_direct_channels(true);
					src.play();

					sources.emplace_back(std::move(src));
				}

				else if (next_event.is<focus_guid>()) {
					auto& e = next_event.get<focus_guid>();

					ensure(currently_played_scene_index != -1);

					auto& scene = preloaded_scenes[currently_played_scene_index];
					scene.testbed.select_character(scene.hypersomnia[e.guid].get_id());
				}

				else if (next_event.is<focus_index>()) {
					auto& e = next_event.get<focus_index>();

					ensure(currently_played_scene_index != -1);

					auto& scene = preloaded_scenes[currently_played_scene_index];
					scene.testbed.select_character(scene.testbed.characters[e.index]);
				}

				else if (next_event.is<set_sfx_gain>()) {
					auto& e = next_event.get<set_sfx_gain>();

					ensure(currently_played_scene_index != -1);

					auto& scene = preloaded_scenes[currently_played_scene_index];
					scene.session.set_master_gain(e.gain);
				}

				++next_played_event_index;
			}
		}

		augs::machine_entropy new_machine_entropy;
		new_machine_entropy.local = window.collect_entropy(!cfg.debug_disable_cursor_clipping);
		process_exit_key(new_machine_entropy.local);

		if (currently_played_scene_index != -1) {
			auto& scene = preloaded_scenes[currently_played_scene_index];
			
			scene.advance_player();
			scene.view(cfg);
		}

		window.swap_buffers();
	}
}