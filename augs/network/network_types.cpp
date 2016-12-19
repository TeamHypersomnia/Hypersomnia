#include <enet/enet.h>
#undef min
#undef max

#include "network_types.h"

namespace augs {
	namespace network {
		bool message::operator==(const message& b) const {
			return std::make_tuple(message_type, address, payload, messages_to_skip)
				== std::make_tuple(b.message_type, b.address, b.payload, b.messages_to_skip);
		}

		endpoint_address::endpoint_address() : ip(0), port(0) {}

		endpoint_address::endpoint_address(const ENetAddress& addr) {
			ip = addr.host;
			port = addr.port;
		}

		std::string endpoint_address::get_readable_ip() const {
			const unsigned char* p = reinterpret_cast<const unsigned char*>(&ip);
			return typesafe_sprintf("%x.%x.%x.%x", int(p[0]), int(p[1]), int(p[2]), int(p[3]));
		}

		unsigned endpoint_address::get_ip() const {
			return ip;
		}

		unsigned short endpoint_address::get_port() const {
			return port;
		}
	}
}