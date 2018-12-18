#pragma once
#include <cstddef>
#include <cstdint>

constexpr std::size_t max_incoming_connections_v = 64;
constexpr std::size_t max_nickname_length_v = 30;
constexpr std::size_t min_nickname_length_v = 3;

using net_time_t = double;
using client_id_type = int;
using channel_id_type = int;

namespace augs {
	namespace network {
		bool init();
		bool deinit();
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