
#if BUILD_ENET
#include <enet/enet.h>
#undef min
#undef max
#endif

#include "network_types.h"

namespace augs {
	namespace network {
		endpoint_address::endpoint_address() : ip(0), port(0) {}

		endpoint_address::endpoint_address(const ENetAddress& addr) {
#if BUILD_ENET
			ip = addr.host;
			port = addr.port;
#else
			(void)addr;
#endif
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