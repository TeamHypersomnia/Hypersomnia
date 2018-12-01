#pragma once
#include "augs/misc/measurements.h"

#include "augs/network/network_types.h"
#include "augs/network/reliable_channel.h"

namespace augs {
	namespace network {
		class client {
			reliable_channel redundancy;

			augs::amount_measurements<std::size_t> sent_size = 1;
			augs::amount_measurements<std::size_t> recv_size = 1;

		public:
			std::vector<message> collect_entropy();

			bool connect(
				const std::string& address, 
				unsigned timeout_ms
			);

			bool post_redundant(const packet& payload);
			bool send_pending_redundant();

			bool send_reliable(const packet& payload);

			bool has_timed_out(
				const float sequence_interval_ms, 
				const float ms
			) const;

			void disconnect();
			void forceful_disconnect();

			bool is_connected() const;

			unsigned total_bytes_sent() const;
			unsigned total_bytes_received() const;
			unsigned total_packets_sent() const;
			unsigned total_packets_received() const;

			std::string format_transmission_details() const;
		};
	}
}
