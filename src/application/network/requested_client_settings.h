#pragma once
#include "augs/network/network_types.h"
#include "augs/misc/constant_size_string.h"
#include "application/setups/client/client_vars.h"

struct public_client_settings {
	vec2 mouse_sensitivity;
};

struct requested_client_settings {
	static constexpr std::size_t buf_len = max_nickname_length_v + 1;

	augs::constant_size_string<buf_len> chosen_nickname;

	public_client_settings public_settings;
	client_net_vars net;
};
