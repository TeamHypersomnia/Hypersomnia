#pragma once
#include "augs/network/network_types.h"

struct requested_client_settings {
	static constexpr std::size_t buf_len = max_nickname_length_v + 1;

	char chosen_nickname[buf_len];
};
