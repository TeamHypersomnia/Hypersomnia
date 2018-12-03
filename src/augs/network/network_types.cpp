#include "augs/network/network_types.h"

bool InitializeYojimbo();
void ShutdownYojimbo();

namespace augs {
	namespace network {
		bool init() {
			return InitializeYojimbo();
		}

		bool deinit() {
			ShutdownYojimbo();
			return true;
		}
	}
}