#include <enet/enet.h>
#undef min
#undef max

#include "network_types.h"

namespace augs {
	namespace network {
		endpoint_address::endpoint_address() : ip(0), port(0) {}

		endpoint_address::endpoint_address(const ENetAddress& addr) {
			ip = addr.host;
			port = addr.port;
		}

		unsigned endpoint_address::get_ip() const {
			return ip;
		}

		unsigned short endpoint_address::get_port() const {
			return port;
		}
	}
}