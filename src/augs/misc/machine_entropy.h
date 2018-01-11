#pragma once
#include <vector>

#include "augs/window_framework/event.h"
#include "augs/network/network_types.h"

namespace augs {
	using local_entropy = std::vector<event::change>;
	using remote_entropy = std::vector<event::change>;
}