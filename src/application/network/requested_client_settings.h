#pragma once
#include "augs/network/network_types.h"
#include "augs/misc/constant_size_string.h"
#include "application/setups/client/client_vars.h"

struct public_client_settings {
	// GEN INTROSPECTOR struct public_client_settings
	vec2 mouse_sensitivity;
	// END GEN INTROSPECTOR
};

struct requested_client_settings {
	static constexpr std::size_t nick_buf_len = max_nickname_length_v + 1;
	static constexpr std::size_t rcon_buf_len = max_rcon_password_length_v + 1;

	// GEN INTROSPECTOR struct requested_client_settings
	augs::constant_size_string<nick_buf_len> chosen_nickname;
	augs::constant_size_string<rcon_buf_len> rcon_password;

	public_client_settings public_settings;
	client_net_vars net;
	// END GEN INTROSPECTOR
};
