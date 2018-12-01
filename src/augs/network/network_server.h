#pragma once
#include <unordered_map>

#include "augs/templates/maybe.h"

#include "augs/network/network_types.h"
#include "augs/network/reliable_channel.h"
#include "augs/network/server_listen_input.h"

namespace augs {
	namespace network {
		class server {
			endpoint_address address;

			struct peer {
				reliable_channel redundancy;
			};

			std::unordered_map<endpoint_address, peer> peer_map;

		public:

			std::vector<message> collect_entropy();

			bool listen(const server_listen_input&);

			// void enable_lag(float loss, unsigned short latency, unsigned short jitter);
			
			bool post_redundant(packet&&, const endpoint_address& target);
			
			bool send_pending_redundant();

			bool has_endpoint(const endpoint_address&) const;
			bool has_timed_out(const endpoint_address&, const float sequence_interval_ms, const float ms = 2000) const;
			
			bool send_reliable(const packet&, const endpoint_address& target);
			
			void disconnect(const endpoint_address& target);
			void forceful_disconnect(const endpoint_address& target);

			//void close_connection(const RakNet::RakNetGUID&, int disconnection_notification_priority);
			//void shutdown(unsigned block_duration, int disconnection_notification_priority);
		};
	}
}