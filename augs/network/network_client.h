#pragma once
#include "network_types.h"
#include "enet_raii.h"

struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;

namespace augs {
	namespace network {
		class client {
			endpoint_address address;
			ENetHost_raii host;
			ENetPeer* peer = nullptr;

		public:
			std::vector<message> collect_entropy();

			bool connect(std::string host, unsigned short port, unsigned timeout_ms);

			bool send_unreliable(const packet& payload);
			bool send_reliable(const packet& payload);
		};
	}
}
