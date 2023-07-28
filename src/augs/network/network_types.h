#pragma once
#include <cstddef>
#include <cstdint>
#include "augs/misc/constant_size_string.h"
#include "augs/network/port_type.h"

constexpr std::size_t max_avatar_bytes_v = 64 * 1024;
constexpr std::size_t max_avatar_side_v = 80;

constexpr std::size_t max_incoming_connections_v = 64;

/* One for the machine admin */
constexpr std::size_t max_mode_players_v = max_incoming_connections_v + 1;

constexpr std::size_t max_rcon_password_length_v = 30;
constexpr std::size_t max_chat_message_length_v = 180;

constexpr std::size_t min_nickname_length_v = 1;
constexpr std::size_t max_nickname_length_v = 30;
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

using server_name_type = augs::constant_size_string<max_server_name_length_v>;
using game_mode_name_type = augs::constant_size_string<max_game_mode_name_length_v>;
using arena_identifier = augs::constant_size_string<max_arena_name_length_v>;
using address_string_type = augs::constant_size_string<max_address_string_length_v>;

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
using client_id_type = int;
using channel_id_type = int;

constexpr client_id_type dead_client_id_v = -1;

namespace yojimbo {
	class Message;
}

namespace augs {
	namespace network {
		bool init();
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