#include <enet/enet.h>
#undef min
#undef max
#include "enet_raii.h"

namespace augs {
	namespace network {
		ENetHost_raii::ENetHost_raii() : ptr(nullptr) {}

		ENetHost_raii::~ENetHost_raii() {
			if (ptr != nullptr) {
				enet_host_destroy(ptr);
			}
		}

		ENetHost* ENetHost_raii::get() {
			return ptr;
		}

		const ENetHost* ENetHost_raii::get() const {
			return ptr;
		}
	}
}