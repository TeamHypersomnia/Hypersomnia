#pragma once
#include <string>

class game_window;

struct config_values {
	int launch_mode = 0;

	unsigned determinism_test_cloned_cosmoi_count = 0;

	bool debug_disable_cursor_clipping = false;

	std::string connect_address;
	unsigned short connect_port = 0;
	unsigned short server_port = 0;
	unsigned short alternative_port = 0;

	std::string nickname;

	unsigned tickrate = 0;

	unsigned jitter_buffer_ms = 0;
	unsigned client_commands_jitter_buffer_ms = 0;

	double interpolation_speed = 0;

	int test_var = 0;
	bool test_randomize_entropies_in_client_setup = 0;
	unsigned test_randomize_entropies_in_client_setup_once_every_steps = 0;

	bool server_launch_http_daemon = 0;
	unsigned short server_http_daemon_port = 0;
	std::string server_http_daemon_html_file_path;

	void get_values(game_window&);
};