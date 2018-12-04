#pragma once
#include <cstddef>

constexpr std::size_t max_incoming_connections_v = 64;
using net_time_t = double;
using client_id_type = int;

namespace augs {
	namespace network {
		bool init();
		bool deinit();
	}
}