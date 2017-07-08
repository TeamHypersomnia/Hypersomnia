
#if BUILD_ENET
#include <enet/enet.h>
#undef min
#undef max
#endif
#include "enet_raii.h"

namespace augs {
	namespace network {
		ENetHost_raii::~ENetHost_raii() {
#if BUILD_ENET
			if (ptr != nullptr) {
				enet_host_destroy(ptr);
			}
#endif
		}

		ENetHost* ENetHost_raii::get() {
			return ptr;
		}

		const ENetHost* ENetHost_raii::get() const {
			return ptr;
		}
	}
}