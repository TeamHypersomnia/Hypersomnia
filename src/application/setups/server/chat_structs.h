#pragma once
#include "augs/network/network_types.h"
#include "game/modes/mode_player_id.h"

enum class chat_target_type : unsigned char {
	GENERAL,
	TEAM_ONLY,

	INFO,
	INFO_CRITICAL,

	COUNT
};

struct client_requested_chat {
	static constexpr auto buf_len = max_chat_message_length_v;

	chat_target_type target;
	augs::constant_size_string<buf_len> message;
};

struct server_broadcasted_chat {
	static constexpr auto buf_len = max_chat_message_length_v;

	mode_player_id author;
	chat_target_type target;
	augs::constant_size_string<buf_len> message;
};
