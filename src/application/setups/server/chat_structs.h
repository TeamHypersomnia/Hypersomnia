#pragma once
#include "augs/misc/constant_size_string.h"
#include "augs/network/network_types.h"
#include "game/modes/mode_player_id.h"
#include "game/enums/faction_type.h"
#include "game/modes/session_id.h"

enum class chat_target_type : unsigned char {
	GENERAL,
	TEAM_ONLY,

	INFO,
	INFO_CRITICAL,

	KICK,
	BAN,

	DOWNLOADING_FILES,
	FINISHED_DOWNLOADING,

	SERVER_SHUTTING_DOWN,

	COUNT
};

struct client_requested_chat {
	static constexpr auto buf_len = max_chat_message_length_v;

	chat_target_type target;
	augs::constant_size_string<buf_len> message;
};

struct server_broadcasted_chat {
	static constexpr auto buf_len = max_chat_message_length_v;

	session_id_type author = session_id_type::dead();
	chat_target_type target;
	augs::constant_size_string<buf_len> message;
	bool recipient_shall_kindly_leave = false;
};
