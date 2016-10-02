#include "config_values.h"
#include "game_window.h"

#define NVP(x) x, #x

void config_values::get_values(game_window& w) {
	auto set = [&w](auto& c, const std::string& ss) { w.get_config_value(c, ss); };

	set(NVP(launch_mode));

	set(NVP(determinism_test_cloned_cosmoi_count));

	set(NVP(debug_disable_cursor_clipping));

	set(NVP(connect_address));
	set(NVP(connect_port));
	set(NVP(server_port));
	set(NVP(alternative_port));

	set(NVP(nickname));

	set(NVP(tickrate));

	set(NVP(jitter_buffer_ms));
	set(NVP(client_commands_jitter_buffer_ms));

	set(NVP(interpolation_speed));

	set(NVP(test_var));
	set(NVP(test_randomize_entropies_in_client_setup));
	set(NVP(test_randomize_entropies_in_client_setup_once_every_steps));

	set(NVP(server_launch_http_daemon));
	set(NVP(server_http_daemon_port));
	set(NVP(server_http_daemon_html_file_path));
}
