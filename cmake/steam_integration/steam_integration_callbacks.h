#pragma once
#include <vector>
#include <variant>
#include <string>

struct steam_new_url_launch_parameters {};

struct steam_change_server_request {
	std::string server_address;
	std::string server_password;
};

struct steam_new_join_game_request {
	std::string connect_cli;
};

using steam_callback_variant_type = std::variant<
	steam_new_url_launch_parameters,
	steam_new_join_game_request,
	steam_change_server_request
>;

using steam_callback_queue_type = std::vector<
	steam_callback_variant_type
>;


