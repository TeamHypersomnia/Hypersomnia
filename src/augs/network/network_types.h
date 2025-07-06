#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include "augs/misc/constant_size_string.h"
#include "augs/network/port_type.h"

/*
	Dhp XX, 273 (Thanissaro)

	8 - Of paths, the eightfold is best.
	4 - Of truths, the four sayings.
	1 - Of qualities, dispassion.
	2 - Of two-footed beings,
		the one with the eyes
		to see.
*/
constexpr port_type DEFAULT_GAME_PORT_V = 8412;

constexpr port_type DEFAULT_MASTERSERVER_PORT_V = 8430;
constexpr const char* demo_address_preffix_v = "demo://";

constexpr std::size_t max_avatar_bytes_v = 64 * 1024;
constexpr std::size_t max_avatar_side_v = 80;

constexpr std::size_t max_incoming_connections_v = 64;
constexpr std::size_t integrated_client_id_v = max_incoming_connections_v;

constexpr std::size_t max_bot_quota_v = 20u;

/* One for the integrated_client_id_v admin */
constexpr std::size_t max_mode_humans_v = max_incoming_connections_v + 1;

constexpr std::size_t max_mode_players_v = max_mode_humans_v + max_bot_quota_v;

constexpr std::size_t max_rcon_password_length_v = 30;
constexpr std::size_t max_chat_message_length_v = 180;

constexpr std::size_t min_nickname_length_v = 1;
constexpr std::size_t max_nickname_length_v = 40;
constexpr std::size_t max_clan_length_v = 20;
constexpr std::size_t max_server_name_length_v = 60;

constexpr std::size_t arena_public_key_length_v = 80;
constexpr std::size_t max_arena_name_length_v = 30;
constexpr std::size_t max_game_mode_name_length_v = 30;

constexpr std::size_t max_arena_file_path_length_v = 70;
constexpr std::size_t max_version_timestamp_length_v = 70;
constexpr std::size_t max_total_file_path_length_v = 250;

constexpr std::size_t max_arenas_in_pool_v = 50;

constexpr std::size_t max_block_size_v = 2 * 1024 * 1024; // 2 MB
constexpr std::size_t block_fragment_size_v = 1 * 1024;
constexpr std::size_t max_packet_size_v = 4 * 1024;

constexpr std::size_t max_address_string_length_v = 255;
constexpr std::size_t default_max_std_string_length_v = 1024 * 8;

constexpr std::size_t max_signalling_udp_message_length_v = 1024 * 4;

using server_name_type = augs::constant_size_string<max_server_name_length_v>;
using game_mode_name_type = augs::constant_size_string<max_game_mode_name_length_v>;
using arena_identifier = augs::constant_size_string<max_arena_name_length_v>;
using arena_and_mode_identifier = augs::constant_size_string<max_arena_name_length_v + max_game_mode_name_length_v + 1>;
using address_string_type = augs::constant_size_string<max_address_string_length_v>;
using host_string_type = augs::constant_size_string<50>;
using webrtc_id_type = address_string_type;
using hour_and_minute_str = augs::constant_size_string<5>;
using signalling_message_str = augs::constant_size_string<max_signalling_udp_message_length_v>;

using client_nickname_type = augs::constant_size_string<max_nickname_length_v>;
using version_timestamp_string = augs::constant_size_string<max_version_timestamp_length_v>;

inline bool nickname_len_in_range(const std::size_t len) {
	return len >= min_nickname_length_v && len <= max_nickname_length_v;
}

inline bool is_wrong_whitespace(const char c) {
	return std::isspace(static_cast<unsigned char>(c)) && c != ' ';
}

template <class T>
inline bool is_nickname_valid_characters(const T& nickname) {
	const bool only_spaces = std::count(nickname.begin(), nickname.end(), ' ') == static_cast<long>(nickname.length());

	if (only_spaces) {
		return false;
	}

	for (const auto& c : nickname) {
		if (c == '\0') {
			return false;
		}

		if (is_wrong_whitespace(c)) {
			return false;
		}
	}

	return true;
}

using net_time_t = double;
using client_id_type = uint8_t;
using channel_id_type = int;

constexpr client_id_type dead_client_id_v = -1;

namespace yojimbo {
	class Message;
}

namespace augs {
	namespace network {
		bool init(bool rtc_errors, bool);
		bool deinit();
		void enable_detailed_logs(bool);
	}
}

enum class message_handler_result {
	ABORT_AND_DISCONNECT,
	CONTINUE
};

struct network_info {
	float rtt_ms;
	float loss_percent;

	float sent_kbps;
	float received_kbps;
	float acked_kbps;

	uint64_t packets_sent;
	uint64_t packets_received;
	uint64_t packets_acked;

	bool are_set() const {
		return packets_sent > 0 && packets_received > 0;
	}
};

struct server_network_info {
	float sent_kbps = 0.f;
	float received_kbps = 0.f;

	bool are_set() const {
		return sent_kbps > 0.f && received_kbps > 0;
	}
};