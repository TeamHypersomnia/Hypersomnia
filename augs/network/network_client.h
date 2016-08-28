#pragma once
#include "network_types.h"
#include "enet_raii.h"
#include "reliable_channel.h"

#include "augs/misc/measurements.h"

struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;

namespace augs {
	namespace network {
		class client {
			endpoint_address address;
			ENetHost_raii host;
			ENetPeer* peer = nullptr;

			reliable_channel redundancy;

			augs::measurements sent_size = augs::measurements(L"", false, 1);
			augs::measurements recv_size = augs::measurements(L"", false, 1);
		public:
			std::vector<message> collect_entropy();

			bool connect(std::string host, unsigned short port, unsigned timeout_ms);

			bool post_redundant(const packet& payload);
			bool send_pending_redundant();

			bool send_reliable(const packet& payload);

			unsigned total_bytes_sent() const;
			unsigned total_bytes_received() const;
			unsigned total_packets_sent() const;
			unsigned total_packets_received() const;

			std::string format_transmission_details() const;
		};
	}
}
