#include "augs/network/network_types.h"

bool InitializeYojimbo();
void ShutdownYojimbo();

namespace augs {
	namespace network {
		bool endpoint_address::operator==(const endpoint_address& b) const {
			// TODO_NET
			(void)b;
			return true;
		}

		std::string endpoint_address::get_readable() const {
			return "1";
		}

		bool init() {
			return InitializeYojimbo();
		}

		bool deinit() {
			ShutdownYojimbo();
			return true;
		}
	}
}