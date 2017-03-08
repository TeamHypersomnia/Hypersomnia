#include "augs/templates/maybe_const.h"

namespace augs {
	template <bool C, class F>
	void introspect(
		maybe_const_ref_t<C, > t,
		F f
	) {
		f(t.NVP(launch_mode));
		f(t.NVP(input_recording_mode));

		f(t.NVP(recording_replay_speed));

		f(t.NVP(determinism_test_cloned_cosmoi_count));

		f(t.NVP(check_content_integrity_every_launch));
		f(t.NVP(save_regenerated_atlases_as_binary));
		f(t.NVP(debug_regenerate_content_every_launch));

		f(t.NVP(enable_hrtf));
		f(t.NVP(max_number_of_sound_sources));

		f(t.NVP(audio_output_device));

		f(t.NVP(sound_effects_volume));
		f(t.NVP(music_volume));

		f(t.NVP(debug_disable_cursor_clipping));

		f(t.NVP(connect_address));
		f(t.NVP(connect_port));
		f(t.NVP(server_port));
		f(t.NVP(alternative_port));

		f(t.NVP(nickname));
		f(t.NVP(debug_second_nickname));

		f(t.NVP(mouse_sensitivity));

		f(t.NVP(tickrate));

		f(t.NVP(jitter_buffer_ms));
		f(t.NVP(client_commands_jitter_buffer_ms));

		f(t.NVP(interpolation_speed));
		f(t.NVP(misprediction_smoothing_multiplier));

		f(t.NVP(debug_var));
		f(t.NVP(debug_randomize_entropies_in_client_setup));
		f(t.NVP(debug_randomize_entropies_in_client_setup_once_every_steps));

		f(t.NVP(server_launch_http_daemon));
		f(t.NVP(server_http_daemon_port));
		f(t.NVP(server_http_daemon_html_file_path));

		f(t.NVP(db_path));
		f(t.NVP(survey_num_file_path));
		f(t.NVP(post_data_file_path));
		f(t.NVP(last_session_update_link));

		f(t.NVP(director_scenario_filename));
		f(t.NVP(menu_intro_scenario_filename));

		f(t.NVP(menu_theme_filename));

		f(t.NVP(rewind_intro_scene_by_secs));
		f(t.NVP(start_menu_music_at_secs));

		f(t.NVP(skip_credits));
		f(t.NVP(latest_news_url));
	}

}