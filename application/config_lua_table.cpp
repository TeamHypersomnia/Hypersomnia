#include "config_lua_table.h"
#include "game_window.h"

#include <luabind/luabind.hpp>
#include "game/bindings/bind_game_and_augs.h"
#include "augs/log.h"

#include "augs/templates/string_templates.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/view/viewing_session.h"

#include "game/detail/gui/character_gui.h"
#include "augs/scripting/lua_state_raii.h"

#define NVP(x) x, #x

template<class T>
static void get_config_value(augs::lua_state_raii& lua, const std::string& field, T& into) {
	into = luabind::object_cast<T>(luabind::globals(lua)["config_table"][field]);
}

static void get_config_value(augs::lua_state_raii& lua, const std::string& field, bool& into) {
	into = luabind::object_cast<double>(luabind::globals(lua)["config_table"][field]) > 0.0;
}

void config_lua_table::get_values(augs::lua_state_raii& lua) {
	auto set = [&](auto& c, const std::string& ss) { 
		get_config_value(lua, replace_all(ss, ".", "_"), c); 
	};

	set(NVP(launch_mode));
	set(NVP(input_recording_mode));

	set(NVP(recording_replay_speed));

	set(NVP(check_content_integrity_every_launch));
	set(NVP(save_regenerated_atlases_as_binary));
	set(NVP(debug_regenerate_content_every_launch));

	set(NVP(determinism_test_cloned_cosmoi_count));
	
	set(NVP(enable_hrtf));
	set(NVP(max_number_of_sound_sources));

	set(NVP(audio_output_device));

	set(NVP(sound_effects_volume));
	set(NVP(music_volume));

	set(NVP(debug_disable_cursor_clipping));

	set(NVP(mouse_sensitivity));

	set(NVP(connect_address));
	set(NVP(connect_port));
	set(NVP(server_port));
	set(NVP(alternative_port));

	set(NVP(nickname));
	set(NVP(debug_second_nickname));

	set(NVP(tickrate));

	set(NVP(jitter_buffer_ms));
	set(NVP(client_commands_jitter_buffer_ms));

	set(NVP(interpolation_speed));
	set(NVP(misprediction_smoothing_multiplier));
	
	set(NVP(debug_var));
	set(NVP(debug_randomize_entropies_in_client_setup));
	set(NVP(debug_randomize_entropies_in_client_setup_once_every_steps));

	set(NVP(server_launch_http_daemon));
	set(NVP(server_http_daemon_port));
	set(NVP(server_http_daemon_html_file_path));

	set(NVP(db_path));
	set(NVP(survey_num_file_path));
	set(NVP(post_data_file_path));
	set(NVP(last_session_update_link));

	set(NVP(director_scenario_filename));
	set(NVP(menu_intro_scenario_filename));

	set(NVP(menu_theme_filename));

	set(NVP(rewind_intro_scene_by_secs));
	set(NVP(start_menu_music_at_secs));

	set(NVP(skip_credits));
	set(NVP(latest_news_url));
	
	set(NVP(hotbar.increase_inside_alpha_when_selected));
	set(NVP(hotbar.colorize_inside_when_selected));
		
	set(NVP(hotbar.primary_selected_color));
	set(NVP(hotbar.secondary_selected_color)); 

}

config_lua_table::launch_type config_lua_table::get_launch_mode() const {
	switch (static_cast<int>(launch_mode)) {
	case 0: return launch_type::MAIN_MENU; break;
	case 1: return launch_type::LOCAL; break;
	case 2: return launch_type::LOCAL_DETERMINISM_TEST; break;
	case 3: return launch_type::DIRECTOR; break;
	case 4: return launch_type::ONLY_CLIENT; break;
	case 5: return launch_type::ONLY_SERVER; break;
	case 6: return launch_type::CLIENT_AND_SERVER; break;
	case 7: return launch_type::TWO_CLIENTS_AND_SERVER; break;
	default: return launch_type::INVALID; break;
	}
}

input_recording_type config_lua_table::get_input_recording_mode() const {
	switch (static_cast<int>(input_recording_mode)) {
	case 0: return input_recording_type::DISABLED; break;
	case 1: return input_recording_type::LIVE_WITH_BUFFER; break;
	case 2: return input_recording_type::LIVE; break;
	default: return input_recording_type::DISABLED; break;
	}
}