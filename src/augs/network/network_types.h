#pragma once
#include <cstddef>

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