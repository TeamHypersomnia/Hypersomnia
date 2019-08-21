#pragma once
#include <cstddef>
#include <cstdint>

constexpr std::size_t max_avatar_bytes_v = 64 * 1024;
constexpr std::size_t max_avatar_side_v = 80;

constexpr std::size_t max_incoming_connections_v = 64;

/* One for the machine admin */
constexpr std::size_t max_mode_players_v = max_incoming_connections_v + 1;

constexpr std::size_t max_nickname_length_v = 30;
constexpr std::size_t max_rcon_password_length_v = 30;
constexpr std::size_t max_chat_message_length_v = 180;
constexpr std::size_t min_nickname_length_v = 3;

inline bool nickname_len_in_range(const std::size_t len) {
	return len >= min_nickname_length_v && len <= max_chat_message_length_v;
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