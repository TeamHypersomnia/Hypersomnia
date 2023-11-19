#pragma once
#include <array>

struct steam_new_url_launch_parameters {
	static constexpr uint32_t index = 0;
};

struct steam_new_join_game_request {
	static constexpr uint32_t index = 1;

	std::array<char, 256> connect_cli;
};

struct steam_change_server_request {
	static constexpr uint32_t index = 2;

	std::array<char, 64> server_address;
	std::array<char, 64> server_password;
};

