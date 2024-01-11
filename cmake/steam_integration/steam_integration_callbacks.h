#pragma once
#include <array>
#include <vector>
#include <cstddef>

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

struct steam_auth_ticket {
	static constexpr uint32_t index = 3;
	static constexpr int ticket_max_length = 2560;

	uint32_t request_id;
	int result;
	std::vector<std::byte> ticket_bytes;

	bool successful() const {
		return result == 1;
	}
};
