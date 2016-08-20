#pragma once
#include <unordered_map>
#include "network_types.h"
#include "enet_raii.h"

struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;

namespace augs {
	namespace network {
		class server {
			endpoint_address address;
			ENetHost_raii host;

			std::unordered_map<unsigned, ENetPeer*> peer_map;

		public:

			std::vector<message> collect_entropy();

			bool listen(unsigned short port, unsigned max_connections);

			// void enable_lag(float loss, unsigned short latency, unsigned short jitter);
			
			bool send_unreliable(const packet&, const endpoint_address& target);
			bool send_reliable(const packet&, const endpoint_address& target);

			//void close_connection(const RakNet::RakNetGUID&, int disconnection_notification_priority);
			//void shutdown(unsigned block_duration, int disconnection_notification_priority);
		};
	}
}