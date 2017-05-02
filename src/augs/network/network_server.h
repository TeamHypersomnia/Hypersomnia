#pragma once
#include <unordered_map>
#include "network_types.h"
#include "enet_raii.h"
#include "reliable_channel.h"

struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;

namespace augs {
	namespace network {
		class server {
			endpoint_address address;
			ENetHost_raii host;

			struct peer {
				ENetPeer* ptr;
				reliable_channel redundancy;

				peer() : ptr(nullptr) {}
				peer(ENetPeer * const ptr ) : ptr(ptr) {}

				operator ENetPeer*() {
					return ptr;
				}

				operator ENetPeer*() const {
					return ptr;
				}
			};

			std::unordered_map<endpoint_address, peer> peer_map;

		public:

			std::vector<message> collect_entropy();

			bool listen(unsigned short port, unsigned max_connections);

			// void enable_lag(float loss, unsigned short latency, unsigned short jitter);
			
			bool post_redundant(const packet&, const endpoint_address& target);
			
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