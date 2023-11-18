#pragma once
#include <vector>
#include <variant>

struct steam_new_url_launch_parameters {};

using steam_callback_variant_type = std::variant<
	steam_new_url_launch_parameters
>;

using steam_callback_queue_type = std::vector<
	steam_callback_variant_type
>;


